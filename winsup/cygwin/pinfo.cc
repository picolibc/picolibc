/* pinfo.cc: process table support

   Copyright 1996, 1997, 1998, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include "winsup.h"

/* The first pid used; also the lowest value allowed. */
#define PBASE 1000

static char NO_COPY pinfo_dummy[sizeof(pinfo)] = {0};

pinfo NO_COPY *myself = (pinfo *)&pinfo_dummy;	// Avoid myself != NULL checks

/* Initialize the process table.
   This is done once when the dll is first loaded.  */

void
pinfo_list::init (void)
{
  next_pid = PBASE;	/* Next pid to try to allocate.  */

  /* We assume the shared data area is already initialized to zeros.
     Note that SIG_DFL is zero.  */
}

pinfo * __stdcall
set_myself (pinfo *p)
{
  myself = p;
  if (!p)
    return NULL;

  myself->start_time = time (NULL); /* Register our starting time. */

  char buf[30];
  __small_sprintf (buf, "cYg%8x %x", _STRACE_INTERFACE_ACTIVATE_ADDR,
		   &strace.active);
  OutputDebugString (buf);

  (void) GetModuleFileName (NULL, myself->progname,
			    sizeof(myself->progname));
  if (strace.active)
    {
      extern char osname[];
      strace.prntf (1, "**********************************************");
      strace.prntf (1, "Program name: %s", myself->progname);
      strace.prntf (1, "App version:  %d.%d, api: %d.%d",
			user_data->dll_major, user_data->dll_minor,
			user_data->api_major, user_data->api_minor);
      strace.prntf (1, "DLL version:  %d.%d, api: %d.%d",
			cygwin_version.dll_major, cygwin_version.dll_minor,
			cygwin_version.api_major, cygwin_version.api_minor);
      strace.prntf (1, "DLL build:    %s", cygwin_version.dll_build_date);
      strace.prntf (1, "OS version:   Windows %s", osname);
      strace.prntf (1, "**********************************************");
    }

  return myself;
}

/* Initialize the process table entry for the current task.
   This is not called for fork'd tasks, only exec'd ones.  */
void __stdcall
pinfo_init (LPBYTE info)
{
  if (info != NULL)
    {
      /* The process was execed.  Reuse entry from the original
	 owner of this pid. */
      environ_init (0);	  /* Needs myself but affects calls below */

      /* spawn has already set up a pid structure for us so we'll use that */

      myself->process_state |= PID_CYGPARENT;

      /* Inherit file descriptor information from parent in info.
       */
      LPBYTE b = dtable.de_linearize_fd_array (info);
      extern char title_buf[];
      if (b && *b)
	old_title = strcpy (title_buf, (char *)b);
    }
  else
    {
      /* Invent our own pid.  */

      if (!set_myself (cygwin_shared->p.allocate_pid ()))
	api_fatal ("No more processes");
      myself->ppid = myself->pgid = myself->sid = myself->pid;
      myself->ctty = -1;
      myself->uid = USHRT_MAX;
      myself->logsrv[0] = '\0';
      myself->domain[0] = '\0';

      environ_init (0);		/* call after myself has been set up */
    }

  debug_printf ("pid %d, pgid %d", myself->pid, myself->pgid);
}

/* [] operator.  This is the mechanism for table lookups.  */
/* Returns the index into the pinfo_list table for pid arg */

pinfo *
pinfo_list::operator[] (pid_t pid)
{
  if (pid <= 0)
    return NULL;

  pinfo *p = vec + (pid % size ());

  if (p->pid != pid || p->process_state == PID_NOT_IN_USE)
    return NULL;
  else
    return p;
}

struct sigaction&
pinfo::getsig(int sig)
{
#ifdef _MT_SAFE
  if ( thread2signal )
    return thread2signal->sigs[sig];
  return sigs[sig];
#else
  return sigs[sig];
#endif
};

sigset_t&
pinfo::getsigmask ()
{
#ifdef _MT_SAFE
  if ( thread2signal )
    return *thread2signal->sigmask;
  return sig_mask;
#else
  return sig_mask;
#endif
};

void
pinfo::setsigmask (sigset_t _mask)
{
#ifdef _MT_SAFE
  if ( thread2signal )
	*(thread2signal->sigmask) = _mask;
  sig_mask=_mask;
#else
  sig_mask=_mask;
#endif
}

LONG *
pinfo::getsigtodo(int sig)
{
#ifdef _MT_SAFE
  if ( thread2signal )
    return thread2signal->sigtodo + __SIGOFFSET + sig;
  return _sigtodo + __SIGOFFSET + sig;
#else
  return _sigtodo + __SIGOFFSET + sig;
#endif
}

extern HANDLE hMainThread;

HANDLE
pinfo::getthread2signal()
{
#ifdef _MT_SAFE
  if ( thread2signal )
    return thread2signal->win32_obj_id;
  return hMainThread;
#else
  return hMainThread;
#endif
}

void
pinfo::setthread2signal(void *_thr)
{
#ifdef _MT_SAFE
   // assert has myself lock
   thread2signal=(ThreadItem*)_thr;
#else
#endif
}

void
pinfo::copysigs(pinfo *_other)
{
  sigs = _other->sigs;
}

pinfo * __stdcall
procinfo (int pid)
{
  return cygwin_shared->p[pid];
}

#ifdef DEBUGGING
/*
 * Code to lock/unlock the process table.
 */

int __stdcall
lpfu (const char *func, int ln, DWORD timeout)
{
  int rc;
  DWORD t;

  debug_printf ("timeout %d, pinfo_mutex %p", timeout, pinfo_mutex);
  t = (timeout == INFINITE) ? 10000 : timeout;
  SetLastError(0);
  while ((rc = WaitForSingleObject (pinfo_mutex, t)) != WAIT_OBJECT_0)
    {
      if (rc == WAIT_ABANDONED_0)
	break;
      system_printf ("%s:%d having problems getting lock", func, ln);
      system_printf ("*** %s, rc %d, %E", cygwin_shared->p.lock_info, rc);
      if (t == timeout)
	break;
     }

  __small_sprintf (cygwin_shared->p.lock_info, "%s(%d), pid %d ", func, ln,
		   (user_data && myself) ? (int)myself->dwProcessId : -1);
  return rc;
}

void
unlock_pinfo (void)
{

  debug_printf ("handle %d", pinfo_mutex);

  if (!cygwin_shared->p.lock_info[0])
    system_printf ("lock_info not set?");
  else
    strcat (cygwin_shared->p.lock_info, " unlocked");
  if (!ReleaseMutex (pinfo_mutex))
    system_printf ("ReleaseMutext (pinfo_mutex<%p>) failed, %E", pinfo_mutex);
}
#else
/*
 * Code to lock/unlock the process table.
 */

int __stdcall
lock_pinfo_for_update (DWORD timeout)
{
  DWORD rc;
  DWORD t;

  debug_printf ("timeout %d, pinfo_mutex %p", timeout, pinfo_mutex);
  t = (timeout == INFINITE) ? 10000 : timeout;
  SetLastError(0);
  while ((rc = WaitForSingleObject (pinfo_mutex, t)) != WAIT_OBJECT_0)
    {
      if (rc == WAIT_ABANDONED_0)
	break;
      system_printf ("rc %d, pinfo_mutex %p, %E", pinfo_mutex, rc);
      if (t == timeout)
	break;
      if (rc == WAIT_FAILED)
	/* sigh, must be properly fixed up later. */
	return rc;
      Sleep(10); /* to prevent 100% CPU in those rare cases */
     }

  return (int)rc;
}

void
unlock_pinfo (void)
{

  debug_printf ("handle %d", pinfo_mutex);

  ReleaseMutex (pinfo_mutex);
}
#endif


/* Allocate a process table entry by finding an empty slot in the
   fixed-size process table.  We could use a linked list, but this
   would probably be too slow.

   Try to allocate next_pid, incrementing next_pid and trying again
   up to size() times at which point we reach the conclusion that
   table is full.  Eventually at this point we would grow the table
   by size() and start over.  If we find a pid to use,

   If all else fails, sweep through the loop looking for processes that
   may have died abnormally without registering themselves as "dead".
   Clear out these pinfo structures.  Then scan the table again.

   Note that the process table is in the shared data space and thus
   is susceptible to corruption.  The amount of time spent scanning the
   table is presumably quite small compared with the total time to
   create a process.
*/

pinfo *
pinfo_list::allocate_pid (void)
{

  pinfo *newp;

  lock_pinfo_for_update (INFINITE);
  for (int tries = 0; ; tries++)
    {
      for (int i = next_pid; i < (next_pid + size ()); i++)
	{
	  /* i mod size() gives place to check */
	  newp = vec + (i % size());
	  if (newp->process_state == PID_NOT_IN_USE)
	    {
	      debug_printf ("found empty slot %d for pid %d",
			     (i % size ()), i);
	      next_pid = i;
	      goto gotit;
	    }
	}

      if (tries > 0)
	break;

      /* try once to remove bogus dead processes */
      debug_printf ("clearing out deadwood");
      for (newp = vec; newp < vec + size(); newp++)
	proc_exists (newp);
    }

  /* The process table is full.  */
  debug_printf ("process table is full");
  unlock_pinfo ();

  return NULL;

gotit:

  /* Set new pid based on the position of this element in the pinfo list */
  newp->pid = next_pid;

  /* Determine next slot to consider, wrapping if we hit the end of
   * the array.  Since allocation involves looping through size () pids,
   * don't allow next_pid to be greater than SHRT_MAX - size ().
   */
  if (next_pid < (SHRT_MAX - size ()))
    next_pid++;
  else
    next_pid = PBASE;

  newp->process_state = PID_IN_USE;
  unlock_pinfo ();

  memset (newp, 0, PINFO_ZERO);
  debug_printf ("pid %d, state %x", newp->pid, newp->process_state);
  return newp;
}

void
pinfo::record_death (int lock)
{
  int unlock = lock ? 0 : lock_pinfo_for_update (999);
  if (dwProcessId == GetCurrentProcessId () && !my_parent_is_alive ())
    {
      process_state = PID_NOT_IN_USE;
      hProcess = NULL;
    }

  if (unlock)
    unlock_pinfo ();
}

/* DOCTOOL-START

<sect1 id="func-cygwin-winpid-to-pid">
  <title>cygwin_winpid_to_pid</title>

  <funcsynopsis>
    <funcdef>extern "C" pid_t
      <function>cygwin_winpid_to_pid</function>
      </funcdef>
      <paramdef>int <parameter>winpid</parameter></paramdef>
  </funcsynopsis>

  <para>Given a windows pid, converts to the corresponding Cygwin
pid, if any.  Returns -1 if windows pid does not correspond to
a cygwin pid.</para>
  <example>
    <title>Example use of cygwin_winpid_to_pid</title>
    <programlisting>
      extern "C" cygwin_winpid_to_pid (int winpid);
      pid_t mypid;
      mypid = cygwin_winpid_to_pid (windows_pid);
    </programlisting>
  </example>
</sect1>

   DOCTOOL-END */

extern "C" pid_t
cygwin_winpid_to_pid (int winpid)
{
  for (int i = 0; i < cygwin_shared->p.size (); i++)
    {
      pinfo *p = &cygwin_shared->p.vec[i];

      if (p->process_state == PID_NOT_IN_USE)
	continue;

      /* FIXME: signed vs unsigned comparison: winpid can be < 0 !!! */
      if (p->dwProcessId == (DWORD)winpid)
	return p->pid;
    }

  set_errno (ESRCH);
  return (pid_t) -1;
}
