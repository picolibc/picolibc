/* pinfo.cc: process table support

   Copyright 1996, 1997, 1998, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdarg.h>
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygerrno.h"
#include "sigproc.h"
#include "pinfo.h"
#include "cygwin_version.h"
#include "perprocess.h"
#include "environ.h"
#include <assert.h>
#include <sys/wait.h>
#include <ntdef.h>
#include "ntdll.h"
#include "shared_info.h"
#include "cygheap.h"
#include "fhandler.h"
#include "cygmalloc.h"
#include "cygtls.h"
#include "child_info.h"

static char NO_COPY pinfo_dummy[sizeof (_pinfo)] = {0};

pinfo NO_COPY myself ((_pinfo *)&pinfo_dummy);	// Avoid myself != NULL checks

/* Initialize the process table.
   This is done once when the dll is first loaded.  */

void __stdcall
set_myself (HANDLE h)
{
  extern child_info *child_proc_info;

  if (!h)
    cygheap->pid = cygwin_pid (GetCurrentProcessId ());
  myself.init (cygheap->pid, PID_IN_USE | PID_MYSELF, h);
  myself->process_state |= PID_IN_USE;
  myself->dwProcessId = GetCurrentProcessId ();

  (void) GetModuleFileName (NULL, myself->progname, sizeof (myself->progname));
  if (!strace.active)
    strace.hello ();
  debug_printf ("myself->dwProcessId %u", myself->dwProcessId);
  myself.initialize_lock ();
  if (h)
    {
      /* here if execed */
      static pinfo NO_COPY myself_identity;
      myself_identity.init (cygwin_pid (myself->dwProcessId), PID_EXECED);
      myself->start_time = time (NULL); /* Register our starting time. */
      myself->exec_sendsig = NULL;
      myself->exec_dwProcessId = 0;
    }
  else if (!myself->wr_proc_pipe)
    myself->start_time = time (NULL); /* Register our starting time. */
  else
    {
      /* We've inherited the parent's wr_proc_pipe.  We don't need it,
	 so close it. */
      if (child_proc_info->parent_wr_proc_pipe)
	CloseHandle (child_proc_info->parent_wr_proc_pipe);
      if (cygheap->pid_handle)
	{
	  ForceCloseHandle (cygheap->pid_handle);
	  cygheap->pid_handle = NULL;
	}
    }
# undef child_proc_info
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

      set_myself (NULL);
      myself->ppid = 1;
      myself->pgid = myself->sid = myself->pid;
      myself->ctty = -1;
      myself->uid = ILLEGAL_UID;
      myself->gid = UNKNOWN_GID;
      environ_init (NULL, 0);	/* call after myself has been set up */
    }

  debug_printf ("pid %d, pgid %d", myself->pid, myself->pgid);
}

void
_pinfo::exit (UINT n, bool norecord)
{
  exit_state = ES_FINAL;
  cygthread::terminate ();
  if (norecord)
    sigproc_terminate ();	/* Just terminate signal and process stuff */
  else
    exitcode = n;		/* We're really exiting.  Record the UNIX exit code. */

  if (this)
    {
      /* FIXME:  There is a potential race between an execed process and its
	 parent here.  I hated to add a mutex just for this, though.  */
      struct rusage r;
      fill_rusage (&r, hMainProc);
      add_rusage (&rusage_self, &r);

      if (!norecord)
	{
	  process_state = PID_EXITED;
	  /* We could just let this happen automatically when the process
	     exits but this should gain us a microsecond or so by notifying
	     the parent early.  */
	  if (wr_proc_pipe)
	    CloseHandle (wr_proc_pipe);
	}
    }

  sigproc_printf ("Calling ExitProcess %d", n);
  _my_tls.stacklock = 0;
  _my_tls.stackptr = _my_tls.stack;
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
  DWORD access = FILE_MAP_READ
		 | (flag & (PID_IN_USE | PID_EXECED | PID_MAP_RW) ? FILE_MAP_WRITE : 0);
  for (int i = 0; i < 10; i++)
    {
      int created;
      char mapname[CYG_MAX_PATH]; /* XXX Not a path */
      shared_name (mapname, "cygpid", n);

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
	  h = OpenFileMapping (access, FALSE, mapname);
	  created = 0;
	}
      else
	{
	  char sa_buf[1024];
	  PSECURITY_ATTRIBUTES sec_attribs =
	    sec_user_nih (sa_buf, cygheap->user.sid(), well_known_world_sid,
			  FILE_MAP_READ);
	  h = CreateFileMapping (INVALID_HANDLE_VALUE, sec_attribs,
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

      procinfo = (_pinfo *) MapViewOfFileEx (h, access, 0, 0, 0, mapaddr);
      if (procinfo)
	/* it worked */;
      else if (exit_state)
	return;		/* exiting */
      else
	{
	  if (GetLastError () == ERROR_INVALID_HANDLE)
	    api_fatal ("MapViewOfFileEx(%p, in_h %p) failed, %E", h, in_h);
	  else
	    {
	      debug_printf ("MapViewOfFileEx(%p, in_h %p) failed, %E", h, in_h);
	      CloseHandle (h);
	    }
	  if (i < 9)
	    continue;
	  else
	    return;
	}

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

void
pinfo::set_acl()
{
  char sa_buf[1024];
  SECURITY_DESCRIPTOR sd;

  sec_acl ((PACL) sa_buf, true, true, cygheap->user.sid (),
	   well_known_world_sid, FILE_MAP_READ);
  if (!InitializeSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION))
    debug_printf ("InitializeSecurityDescriptor %E");
  else if (!SetSecurityDescriptorDacl (&sd, TRUE, (PACL) sa_buf, FALSE))
    debug_printf ("SetSecurityDescriptorDacl %E");
  else if (!SetKernelObjectSecurity (h, DACL_SECURITY_INFORMATION, &sd))
    debug_printf ("SetKernelObjectSecurity %E");
}

void
_pinfo::set_ctty (tty_min *tc, int flags, fhandler_tty_slave *arch)
{
  debug_printf ("checking if /dev/tty%d changed", ctty);
  if ((ctty < 0 || ctty == tc->ntty) && !(flags & O_NOCTTY))
    {
      ctty = tc->ntty;
      syscall_printf ("attached tty%d sid %d, pid %d, tty->pgid %d, tty->sid %d",
		      tc->ntty, sid, pid, pgid, tc->getsid ());

      pinfo p (tc->getsid ());
      if (sid == pid && (!p || p->pid == pid || !proc_exists (p)))
	{
	  paranoid_printf ("resetting tty%d sid.  Was %d, now %d.  pgid was %d, now %d.",
			   tc->ntty, tc->getsid (), sid, tc->getpgid (), pgid);
	  /* We are the session leader */
	  tc->setsid (sid);
	  tc->setpgid (pgid);
	}
      else
	sid = tc->getsid ();
      if (tc->getpgid () == 0)
	tc->setpgid (pgid);
      if (cygheap->ctty != arch)
	{
	  debug_printf ("cygheap->ctty %p, arch %p", cygheap->ctty, arch);
	  if (!cygheap->ctty)
	    syscall_printf ("ctty NULL");
	  else
	    {
	      syscall_printf ("ctty %p, usecount %d", cygheap->ctty,
			      cygheap->ctty->usecount);
	      cygheap->ctty->close ();
	    }
	  cygheap->ctty = arch;
	  if (arch)
	    {
	      arch->usecount++;
	      cygheap->open_fhs++;
	      report_tty_counts (cygheap->ctty, "ctty", "incremented ", "");
	    }
	}
    }
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

  hello_pid = 0;

  if (!ReadFile (__fromthem, &code, sizeof code, &nr, NULL) || nr != sizeof code)
    {
      CloseHandle (hp);
      /* __seterrno ();*/	// this is run from the signal thread, so don't set errno
      goto out;
    }

  switch (code)
    {
    case PICOM_CMDLINE:
      {
	unsigned n = 1;
	CloseHandle (__fromthem); __fromthem = NULL;
	extern int __argc_safe;
	const char *argv[__argc_safe + 1];

	CloseHandle (hp);
	for (int i = 0; i < __argc_safe; i++)
	  {
	    if (IsBadStringPtr (__argv[i], INT32_MAX))
	      argv[i] = "";
	    else
	      argv[i] = __argv[i];
	    n += strlen (argv[i]) + 1;
	  }
	argv[__argc_safe] = NULL;
	if (!WriteFile (__tothem, &n, sizeof n, &nr, NULL))
	  {
	    /*__seterrno ();*/	// this is run from the signal thread, so don't set errno
	    sigproc_printf ("WriteFile sizeof argv failed, %E");
	  }
	else
	  for (const char **a = argv; *a; a++)
	    if (!WriteFile (__tothem, *a, strlen (*a) + 1, &nr, NULL))
	      {
		sigproc_printf ("WriteFile arg %d failed, %E", a - argv);
		break;
	      }
	if (!WriteFile (__tothem, "", 1, &nr, NULL))
	  {
	    sigproc_printf ("WriteFile null failed, %E");
	    break;
	  }
	break;
      }
    case PICOM_FIFO:
      {
	char path[CYG_MAX_PATH + 1];
	unsigned len;
	if (!ReadFile (__fromthem, &len, sizeof len, &nr, NULL)
	    || nr != sizeof len)
	  {
	    CloseHandle (hp);
	    /* __seterrno ();*/	// this is run from the signal thread, so don't set errno
	    goto out;
	  }
	/* Get null-terminated path */
	if (!ReadFile (__fromthem, path, len, &nr, NULL)
	    || nr != len)
	  {
	    CloseHandle (hp);
	    /* __seterrno ();*/	// this is run from the signal thread, so don't set errno
	    goto out;
	  }

	fhandler_fifo *fh = cygheap->fdtab.find_fifo (path);
	HANDLE it[2];
	if (fh == NULL)
	  it[0] = it[1] = NULL;
	else
	  {
	    it[0] = fh->get_handle ();
	    it[1] = fh->get_output_handle ();
	    for (int i = 0; i < 2; i++)
	      if (!DuplicateHandle (hMainProc, it[i], hp, &it[i], 0, false,
				    DUPLICATE_SAME_ACCESS))
		{
		  it[0] = it[1] = NULL;	/* FIXME: possibly left a handle open in child? */
		  break;
		}
	  }

	CloseHandle (hp);
	if (!WriteFile (__tothem, it, sizeof (it), &nr, NULL))
	  {
	    /*__seterrno ();*/	// this is run from the signal thread, so don't set errno
	    sigproc_printf ("WriteFile read handle failed, %E");
	  }

	(void) ReadFile (__fromthem, &nr, sizeof (nr), &nr, NULL);
	break;
      }
    }

out:
  if (__fromthem)
    CloseHandle (__fromthem);
  if (__tothem)
    CloseHandle (__tothem);
}

#define PIPEBUFSIZE (4096 * sizeof (DWORD))

commune_result
_pinfo::commune_send (DWORD code, ...)
{
  HANDLE fromthem = NULL, tome = NULL;
  HANDLE fromme = NULL, tothem = NULL;
  DWORD nr;
  commune_result res;
  va_list args;

  va_start (args, code);

  res.s = NULL;
  res.n = 0;

  if (!this || !pid)
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
      sigproc_printf ("second CreatePipe failed, %E");
      __seterrno ();
      goto err;
    }
  myself.lock ();
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
  for (int i = 0; (isalive = alive ()) && (i < 10000); i++)
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
  switch (code)
    {
    case PICOM_CMDLINE:
      if (!ReadFile (fromthem, &n, sizeof n, &nr, NULL) || nr != sizeof n)
	{
	  __seterrno ();
	  goto err;
	}
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
    case PICOM_FIFO:
      {
	char *path = va_arg (args, char *);
	size_t len = strlen (path) + 1;
	if (!WriteFile (tothem, &len, sizeof (len), &nr, NULL)
	    || nr != sizeof (len))
	  {
	    __seterrno ();
	    goto err;
	  }
	if (!WriteFile (tothem, path, len, &nr, NULL) || nr != len)
	  {
	    __seterrno ();
	    goto err;
	  }

	DWORD x = ReadFile (fromthem, res.handles, sizeof (res.handles), &nr, NULL);
	(void) WriteFile (tothem, &x, sizeof (x), &x, NULL);
	if (!x)
	  goto err;

	if (nr != sizeof (res.handles))
	  {
	    set_errno (EPIPE);
	    goto err;
	  }
	break;
      }
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
  memset (&res, 0, sizeof (res));

out:
  myself->hello_pid = 0;
  myself.unlock ();
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

/* This is the workhorse which waits for the write end of the pipe
   created during new process creation.  If the pipe is closed, it is
   assumed that the cygwin pid has exited.  Otherwise, various "signals"
   can be sent to the parent to inform the parent to perform a certain
   action.

   This code was originally written to eliminate the need for "reparenting"
   but, unfortunately, reparenting is still needed in order to get the
   exit code of an execed windows process.  Otherwise, the exit code of
   a cygwin process comes from the exitcode field in _pinfo. */
static DWORD WINAPI
proc_waiter (void *arg)
{
  extern HANDLE hExeced;
  pinfo& vchild = *(pinfo *) arg;

  siginfo_t si;
  si.si_signo = SIGCHLD;
  si.si_code = SI_KERNEL;
  si.si_pid = vchild->pid;
  si.si_errno = 0;
#if 0	// FIXME: This is tricky to get right
  si.si_utime = pchildren[rc]->rusage_self.ru_utime;
  si.si_stime = pchildren[rc].rusage_self.ru_stime;
#else
  si.si_utime = 0;
  si.si_stime = 0;
#endif
  pid_t pid = vchild->pid;

  for (;;)
    {
      DWORD nb;
      char buf = '\0';
      if (!ReadFile (vchild.rd_proc_pipe, &buf, 1, &nb, NULL)
	  && GetLastError () != ERROR_BROKEN_PIPE)
	{
	  system_printf ("error on read of child wait pipe %p, %E", vchild.rd_proc_pipe);
	  break;
	}

      si.si_uid = vchild->uid;

      switch (buf)
	{
	case __ALERT_ALIVE:
	  continue;
	case 0:
	  /* Child exited.  Do some cleanup and signal myself.  */
	  CloseHandle (vchild.rd_proc_pipe);
	  vchild.rd_proc_pipe = NULL;
	  if (WIFEXITED (vchild->exitcode))
	    si.si_sigval.sival_int = CLD_EXITED;
	  else if (WCOREDUMP (vchild->exitcode))
	    si.si_sigval.sival_int = CLD_DUMPED;
	  else
	    si.si_sigval.sival_int = CLD_KILLED;
	  si.si_status = vchild->exitcode;
	  vchild->process_state = PID_ZOMBIE;
	  break;
	case SIGTTIN:
	case SIGTTOU:
	case SIGTSTP:
	case SIGSTOP:
	  if (ISSTATE (myself, PID_NOCLDSTOP))	// FIXME: No need for this flag to be in _pinfo any longer
	    continue;
	  /* Child stopped.  Signal myself.  */
	  si.si_sigval.sival_int = CLD_STOPPED;
	  break;
	case SIGCONT:
	  continue;
	default:
	  system_printf ("unknown value %d on proc pipe", buf);
	  continue;
	}

      /* Special case:  If the "child process" that died is us, then we're
	 execing.  Just call proc_subproc directly and then exit this loop.
	 We're done here.  */
      if (hExeced)
	{
	  /* execing.  no signals available now. */
	  proc_subproc (PROC_CLEARWAIT, 0);
	  break;
	}

      /* Send a SIGCHLD to myself.   We do this here, rather than in proc_subproc
	 to avoid the proc_subproc lock since the signal thread will eventually
	 be calling proc_subproc and could unnecessarily block. */
      sig_send (myself_nowait, si);

      /* If we're just stopped or got a continue signal, keep looping.
	 Otherwise, return this thread to the pool. */
      if (buf != '\0')
	sigproc_printf ("looping");
      else
	break;
    }

  sigproc_printf ("exiting wait thread for pid %d", pid);
  _my_tls._ctinfo->release ();	/* return the cygthread to the cygthread pool */
  return 0;
}

/* function to set up the process pipe and kick off proc_waiter */
int
pinfo::wait ()
{
  HANDLE out;
  /* FIXME: execed processes should be able to wait for pids that were started
     by the process which execed them. */
  if (!CreatePipe (&rd_proc_pipe, &out, &sec_none_nih, 16))
    {
      system_printf ("Couldn't create pipe tracker for pid %d, %E",
		     (*this)->pid);
      return 0;
    }
  /* Duplicate the write end of the pipe into the subprocess.  Make it inheritable
     so that all of the execed children get it.  */
  if (!DuplicateHandle (hMainProc, out, hProcess, &((*this)->wr_proc_pipe), 0,
			TRUE, DUPLICATE_SAME_ACCESS))
    {
      system_printf ("Couldn't duplicate pipe topid %d(%p), %E", (*this)->pid,
		     hProcess);
      return 0;
    }
  CloseHandle (out);	/* Don't need this end in this proces */

  preserve ();		/* Preserve the shared memory associated with the pinfo */

  /* Fire up a new thread to track the subprocess */
  cygthread *h = new cygthread (proc_waiter, this, "proc_waiter");
  if (!h)
    sigproc_printf ("tracking thread creation failed for pid %d", (*this)->pid);
  else
    {
      wait_thread = h;
      sigproc_printf ("created tracking thread for pid %d, winpid %p, rd_pipe %p",
		      (*this)->pid, (*this)->dwProcessId, rd_proc_pipe);
    }

  return 1;
}

/* function to send a "signal" to the parent when something interesting happens
   in the child. */
bool
pinfo::alert_parent (char sig)
{
  DWORD nb = 0;
  /* Send something to our parent.  If the parent has gone away,
     close the pipe. */
  if (myself->wr_proc_pipe == INVALID_HANDLE_VALUE)
    /* no parent */;
  else if (myself->wr_proc_pipe
	   && WriteFile (myself->wr_proc_pipe, &sig, 1, &nb, NULL))
    /* all is well */;
  else if (GetLastError () != ERROR_BROKEN_PIPE)
    debug_printf ("sending %d notification to parent failed, %E", sig);
  else
    {
      HANDLE closeit = myself->wr_proc_pipe;
      myself->wr_proc_pipe = INVALID_HANDLE_VALUE;
      CloseHandle (closeit);
    }
  return (bool) nb;
}

void
pinfo::release ()
{
  if (h)
    {
#ifdef DEBUGGING
      if (((DWORD) procinfo & 0x77000000) == 0x61000000)
	try_to_debug ();
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

  <funcsynopsis><funcprototype>
    <funcdef>extern "C" pid_t
      <function>cygwin_winpid_to_pid</function>
      </funcdef>
      <paramdef>int <parameter>winpid</parameter></paramdef>
  </funcprototype></funcsynopsis>

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

  pinfolist[nelem].init (cygpid, PID_NOREDIR | (winpid ? PID_ALLPIDS : 0)
			 | pinfo_access);
  if (winpid)
    goto out;

  if (!pinfolist[nelem])
    {
      if (!pinfo_access)
	return;
      pinfolist[nelem].init (cygpid, PID_NOREDIR | (winpid ? PID_ALLPIDS : 0));
      if (!pinfolist[nelem])
	return;
      }

  /* Scan list of previously recorded pids to make sure that this pid hasn't
     shown up before.  This can happen when a process execs. */
  for (unsigned i = 0; i < nelem; i++)
    if (pinfolist[i]->pid == pinfolist[nelem]->pid)
      {
	if ((_pinfo *) pinfolist[nelem] != (_pinfo *) myself)
	  pinfolist[nelem].release ();
	return;
      }

out:
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

void
winpids::set (bool winpid)
{
  __malloc_lock ();
  npids = (this->*enum_processes) (winpid);
  if (pidlist)
    pidlist[npids] = 0;
  __malloc_unlock ();
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
