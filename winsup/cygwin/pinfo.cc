/* pinfo.cc: process table support

   Copyright 1996, 1997, 1998, 2000, 2001, 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygerrno.h"
#include "sigproc.h"
#include "pinfo.h"
#include "cygwin_version.h"
#include "perprocess.h"
#include "environ.h"
#include <assert.h>
#include <ntdef.h>
#include "ntdll.h"
#include "cygthread.h"
#include "shared_info.h"

static char NO_COPY pinfo_dummy[sizeof (_pinfo)] = {0};

pinfo NO_COPY myself ((_pinfo *)&pinfo_dummy);	// Avoid myself != NULL checks

HANDLE hexec_proc;

void __stdcall
pinfo_fixup_after_fork ()
{
  if (hexec_proc)
    CloseHandle (hexec_proc);

  if (!DuplicateHandle (hMainProc, hMainProc, hMainProc, &hexec_proc, 0,
			TRUE, DUPLICATE_SAME_ACCESS))
    {
      system_printf ("couldn't save current process handle %p, %E", hMainProc);
      hexec_proc = NULL;
    }
}

/* Initialize the process table.
   This is done once when the dll is first loaded.  */

void __stdcall
set_myself (pid_t pid, HANDLE h)
{
  DWORD winpid = GetCurrentProcessId ();
  if (pid == 1)
    pid = cygwin_pid (winpid);
  myself.init (pid, PID_IN_USE | PID_MYSELF, h);
  myself->dwProcessId = winpid;
  myself->process_state |= PID_IN_USE;
  myself->start_time = time (NULL); /* Register our starting time. */

  (void) GetModuleFileName (NULL, myself->progname, sizeof (myself->progname));
  if (!strace.active)
    strace.hello ();
  InitializeCriticalSection (&myself->lock);
  return;
}

/* Initialize the process table entry for the current task.
   This is not called for forked tasks, only execed ones.  */
void __stdcall
pinfo_init (char **envp, int envc)
{
  if (envp)
    {
      environ_init (envp, envc);
      /* spawn has already set up a pid structure for us so we'll use that */
      myself->process_state |= PID_CYGPARENT;
    }
  else
    {
      /* Invent our own pid.  */

      set_myself (1);
      myself->ppid = 1;
      myself->pgid = myself->sid = myself->pid;
      myself->ctty = -1;
      myself->uid = ILLEGAL_UID;

      environ_init (NULL, 0);	/* call after myself has been set up */
    }

  debug_printf ("pid %d, pgid %d", myself->pid, myself->pgid);
}

void
_pinfo::exit (UINT n, bool norecord)
{
  if (this)
    {
      if (!norecord)
	process_state = PID_EXITED;

      /* FIXME:  There is a potential race between an execed process and its
	 parent here.  I hated to add a mutex just for this, though.  */
      struct rusage r;
      fill_rusage (&r, hMainProc);
      add_rusage (&rusage_self, &r);
    }

  cygthread::terminate ();
  sigproc_printf ("Calling ExitProcess %d", n);
  ExitProcess (n);
}

void
pinfo::init (pid_t n, DWORD flag, HANDLE in_h)
{
  if (myself && n == myself->pid)
    {
      procinfo = myself;
      destroy = 0;
      h = NULL;
      return;
    }

  void *mapaddr;
  if (!(flag & PID_MYSELF))
    mapaddr = NULL;
  else
    {
      flag &= ~PID_MYSELF;
      HANDLE hdummy;
      mapaddr = open_shared (NULL, 0, hdummy, 0, SH_MYSELF);
    }

  int createit = flag & (PID_IN_USE | PID_EXECED);
  for (int i = 0; i < 10; i++)
    {
      int created;
      char mapname[MAX_PATH];
      __small_sprintf (mapname, "cygpid.%x", n);

      int mapsize;
      if (flag & PID_EXECED)
	mapsize = PINFO_REDIR_SIZE;
      else
	mapsize = sizeof (_pinfo);

      if (in_h)
	{
	  h = in_h;
	  created = 0;
	}
      else if (!createit)
	{
	  h = OpenFileMappingA (FILE_MAP_READ | FILE_MAP_WRITE, FALSE, mapname);
	  created = 0;
	}
      else
	{
	  h = CreateFileMapping (INVALID_HANDLE_VALUE, &sec_all_nih,
				 PAGE_READWRITE, 0, mapsize, mapname);
	  created = h && GetLastError () != ERROR_ALREADY_EXISTS;
	}

      if (!h)
	{
	  if (createit)
	    __seterrno ();
	  procinfo = NULL;
	  return;
	}

      procinfo = (_pinfo *) MapViewOfFileEx (h, FILE_MAP_READ | FILE_MAP_WRITE,
					     0, 0, 0, mapaddr);
      ProtectHandle1 (h, pinfo_shared_handle);

      if ((procinfo->process_state & PID_INITIALIZING) && (flag & PID_NOREDIR)
	  && cygwin_pid (procinfo->dwProcessId) != procinfo->pid)
	{
	  release ();
	  set_errno (ENOENT);
	  return;
	}

      if (procinfo->process_state & PID_EXECED)
	{
	  assert (!i);
	  pid_t realpid = procinfo->pid;
	  debug_printf ("execed process windows pid %d, cygwin pid %d", n, realpid);
	  if (realpid == n)
	    api_fatal ("retrieval of execed process info for pid %d failed due to recursion.", n);
	  n = realpid;
	  release ();
	  if (flag & PID_ALLPIDS)
	    {
	      set_errno (ENOENT);
	      break;
	    }
	  continue;
	}

	/* In certain rare, pathological cases, it is possible for the shared
	   memory region to exist for a while after a process has exited.  This
	   should only be a brief occurrence, so rather than introduce some kind
	   of locking mechanism, just loop.  FIXME: I'm sure I'll regret doing it
	   this way at some point.  */
      if (i < 9 && !created && createit && (procinfo->process_state & PID_EXITED))
	{
	  low_priority_sleep (5);
	  release ();
	  continue;
	}

      if (!created)
	/* nothing */;
      else if (!(flag & PID_EXECED))
	procinfo->pid = n;
      else
	{
	  procinfo->process_state |= PID_IN_USE | PID_EXECED;
	  procinfo->pid = myself->pid;
	}
      break;
    }
  destroy = 1;
}

bool
_pinfo::alive ()
{
  HANDLE h = OpenProcess (PROCESS_QUERY_INFORMATION, false, dwProcessId);
  if (h)
    CloseHandle (h);
  return !!h;
}

extern char **__argv;

void
_pinfo::commune_recv ()
{
  DWORD nr;
  DWORD code;
  HANDLE hp;
  HANDLE __fromthem = NULL;
  HANDLE __tothem = NULL;

  hp = OpenProcess (PROCESS_DUP_HANDLE, false, dwProcessId);
  if (!hp)
    {
      sigproc_printf ("couldn't open handle for pid %d(%u)", pid, dwProcessId);
      hello_pid = -1;
      return;
    }
  if (!DuplicateHandle (hp, fromthem, hMainProc, &__fromthem, 0, false, DUPLICATE_SAME_ACCESS))
    {
      sigproc_printf ("couldn't duplicate fromthem, %E");
      CloseHandle (hp);
      hello_pid = -1;
      return;
    }

  if (!DuplicateHandle (hp, tothem, hMainProc, &__tothem, 0, false, DUPLICATE_SAME_ACCESS))
    {
      sigproc_printf ("couldn't duplicate tothem, %E");
      CloseHandle (__fromthem);
      CloseHandle (hp);
      hello_pid = -1;
      return;
    }

  CloseHandle (hp);
  hello_pid = 0;

  if (!ReadFile (__fromthem, &code, sizeof code, &nr, NULL) || nr != sizeof code)
    {
      /* __seterrno ();*/	// this is run from the signal thread, so don't set errno
      goto out;
    }

  switch (code)
    {
    case PICOM_CMDLINE:
      {
	unsigned n = 1;
	CloseHandle (__fromthem); __fromthem = NULL;
	for (char **a = __argv; *a; a++)
	  n += strlen (*a) + 1;
	if (!WriteFile (__tothem, &n, sizeof n, &nr, NULL))
	  {
	    /*__seterrno ();*/	// this is run from the signal thread, so don't set errno
	    sigproc_printf ("WriteFile sizeof argv failed, %E");
	  }
	else
	  for (char **a = __argv; *a; a++)
	    if (!WriteFile (__tothem, *a, strlen (*a) + 1, &nr, NULL))
	      {
		sigproc_printf ("WriteFile arg %d failed, %E", a - __argv);
		break;
	      }
	  if (!WriteFile (__tothem, "", 1, &nr, NULL))
	    {
	      sigproc_printf ("WriteFile null failed, %E");
	      break;
	    }
      }
    }

out:
  if (__fromthem)
    CloseHandle (__fromthem);
  if (__tothem)
    CloseHandle (__tothem);
}

#define PIPEBUFSIZE (16 * sizeof (DWORD))

commune_result
_pinfo::commune_send (DWORD code)
{
  HANDLE fromthem = NULL, tome = NULL;
  HANDLE fromme = NULL, tothem = NULL;
  DWORD nr;
  commune_result res;

  res.s = NULL;
  res.n = 0;

  if (!pid || !this)
    {
      set_errno (ESRCH);
      goto err;
    }
  if (!CreatePipe (&fromthem, &tome, &sec_all_nih, PIPEBUFSIZE))
    {
      sigproc_printf ("first CreatePipe failed, %E");
      __seterrno ();
      goto err;
    }
  if (!CreatePipe (&fromme, &tothem, &sec_all_nih, PIPEBUFSIZE))
    {
      sigproc_printf ("first CreatePipe failed, %E");
      __seterrno ();
      goto err;
    }
  EnterCriticalSection (&myself->lock);
  myself->tothem = tome;
  myself->fromthem = fromme;
  myself->hello_pid = pid;
  if (!WriteFile (tothem, &code, sizeof code, &nr, NULL) || nr != sizeof code)
    {
      __seterrno ();
      goto err;
    }

  if (sig_send (this, __SIGCOMMUNE))
    goto err;

  /* FIXME: Need something better than an busy loop here */
  bool isalive;
  while ((isalive = alive ()))
    if (myself->hello_pid <= 0)
      break;
    else
      low_priority_sleep (0);

  CloseHandle (tome);
  tome = NULL;
  CloseHandle (fromme);
  fromme = NULL;

  if (!isalive)
    {
      set_errno (ESRCH);
      goto err;
    }

  if (myself->hello_pid < 0)
    {
      set_errno (ENOSYS);
      goto err;
    }

  size_t n;
  if (!ReadFile (fromthem, &n, sizeof n, &nr, NULL) || nr != sizeof n)
    {
      __seterrno ();
      goto err;
    }
  switch (code)
    {
    case PICOM_CMDLINE:
      res.s = (char *) malloc (n);
      char *p;
      for (p = res.s; ReadFile (fromthem, p, n, &nr, NULL); p += nr)
	continue;
      if ((unsigned) (p - res.s) != n)
	{
	  __seterrno ();
	  goto err;
	}
      res.n = n;
      break;
    }
  CloseHandle (tothem);
  CloseHandle (fromthem);
  goto out;

err:
  if (tome)
    CloseHandle (tome);
  if (fromthem)
    CloseHandle (fromthem);
  if (tothem)
    CloseHandle (tothem);
  if (fromme)
    CloseHandle (fromme);
  res.n = 0;
out:
  myself->hello_pid = 0;
  LeaveCriticalSection (&lock);
  return res;
}

char *
_pinfo::cmdline (size_t& n)
{
  char *s;
  if (!this || !pid)
    return NULL;
  if (pid != myself->pid)
    {
      commune_result cr = commune_send (PICOM_CMDLINE);
      s = cr.s;
      n = cr.n;
    }
  else
    {
      n = 1;
      for (char **a = __argv; *a; a++)
	n += strlen (*a) + 1;
      char *p;
      p = s = (char *) malloc (n);
      for (char **a = __argv; *a; a++)
	{
	  strcpy (p, *a);
	  p = strchr (p, '\0') + 1;
	}
      *p = '\0';
    }
  return s;
}

void
pinfo::release ()
{
  if (h)
    {
#ifdef DEBUGGING
      if (((DWORD) procinfo & 0x77000000) == 0x61000000) try_to_debug ();
#endif
      UnmapViewOfFile (procinfo);
      procinfo = NULL;
      ForceCloseHandle1 (h, pinfo_shared_handle);
      h = NULL;
    }
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
  pinfo p (cygwin_pid (winpid));
  if (p)
    return p->pid;

  set_errno (ESRCH);
  return (pid_t) -1;
}

#include <tlhelp32.h>

#define slop_pidlist 200
#define size_pidlist(i) (sizeof (pidlist[0]) * ((i) + 1))
#define size_pinfolist(i) (sizeof (pinfolist[0]) * ((i) + 1))

inline void
winpids::add (DWORD& nelem, bool winpid, DWORD pid)
{
  pid_t cygpid = cygwin_pid (pid);
  if (nelem >= npidlist)
    {
      npidlist += slop_pidlist;
      pidlist = (DWORD *) realloc (pidlist, size_pidlist (npidlist + 1));
      pinfolist = (pinfo *) realloc (pinfolist, size_pinfolist (npidlist + 1));
    }

  pinfolist[nelem].init (cygpid, PID_NOREDIR | (winpid ? PID_ALLPIDS : 0));
  if (winpid)
    /* nothing to do */;
  else if (!pinfolist[nelem])
    return;
  else
    /* Scan list of previously recorded pids to make sure that this pid hasn't
       shown up before.  This can happen when a process execs. */
    for (unsigned i = 0; i < nelem; i++)
      if (pinfolist[i]->pid == pinfolist[nelem]->pid)
	{
	  if ((_pinfo *) pinfolist[nelem] != (_pinfo *) myself)
	    pinfolist[nelem].release ();
	  return;
	}

  pidlist[nelem++] = pid;
}

DWORD
winpids::enumNT (bool winpid)
{
  static DWORD szprocs;
  static SYSTEM_PROCESSES *procs;

  DWORD nelem = 0;
  if (!szprocs)
    procs = (SYSTEM_PROCESSES *) malloc (sizeof (*procs) + (szprocs = 200 * sizeof (*procs)));

  NTSTATUS res;
  for (;;)
    {
      res = NtQuerySystemInformation (SystemProcessesAndThreadsInformation,
				      procs, szprocs, NULL);
      if (res == 0)
	break;

      if (res == STATUS_INFO_LENGTH_MISMATCH)
	procs =  (SYSTEM_PROCESSES *) realloc (procs, szprocs += 200 * sizeof (*procs));
      else
	{
	  system_printf ("error %p reading system process information", res);
	  return 0;
	}
    }

  SYSTEM_PROCESSES *px = procs;
  for (;;)
    {
      if (px->ProcessId)
	add (nelem, winpid, px->ProcessId);
      if (!px->NextEntryDelta)
	break;
      px = (SYSTEM_PROCESSES *) ((char *) px + px->NextEntryDelta);
    }

  return nelem;
}

DWORD
winpids::enum9x (bool winpid)
{
  DWORD nelem = 0;

  HANDLE h = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
  if (!h)
    {
      system_printf ("Couldn't create process snapshot, %E");
      return 0;
    }

  PROCESSENTRY32 proc;
  proc.dwSize = sizeof (proc);

  if (Process32First (h, &proc))
    do
      {
	if (proc.th32ProcessID)
	  add (nelem, winpid, proc.th32ProcessID);
      }
    while (Process32Next (h, &proc));

  CloseHandle (h);
  return nelem;
}

NO_COPY CRITICAL_SECTION winpids::cs;

void
winpids::set (bool winpid)
{
  EnterCriticalSection (&cs);
  npids = (this->*enum_processes) (winpid);
  if (pidlist)
    pidlist[npids] = 0;
  LeaveCriticalSection (&cs);
}

void
winpids::init ()
{
  InitializeCriticalSection (&cs);
}

DWORD
winpids::enum_init (bool winpid)
{
  if (wincap.is_winnt ())
    enum_processes = &winpids::enumNT;
  else
    enum_processes = &winpids::enum9x;

  return (this->*enum_processes) (winpid);
}

void
winpids::release ()
{
  for (unsigned i = 0; i < npids; i++)
    if (pinfolist[i] && (_pinfo *) pinfolist[i] != (_pinfo *) myself)
      pinfolist[i].release ();
}

winpids::~winpids ()
{
  if (npidlist)
    {
      release ();
      free (pidlist);
      free (pinfolist);
    }
}
