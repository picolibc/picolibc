/* pinfo.cc: process table support

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <stdlib.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "sigproc.h"
#include "pinfo.h"
#include "perprocess.h"
#include "environ.h"
#include "ntdll.h"
#include "shared_info.h"
#include "cygheap.h"
#include "cygmalloc.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include "child_info.h"

class pinfo_basic: public _pinfo
{
public:
  pinfo_basic();
};

pinfo_basic::pinfo_basic ()
{
  pid = dwProcessId = GetCurrentProcessId ();
  PWCHAR pend = wcpncpy (progname, global_progname,
			 sizeof (progname) / sizeof (WCHAR) - 1);
  *pend = L'\0';
  /* Default uid/gid are needed very early to initialize shared user info. */
  uid = ILLEGAL_UID;
  gid = ILLEGAL_GID;
}

pinfo_basic myself_initial NO_COPY;

pinfo NO_COPY myself (static_cast<_pinfo *> (&myself_initial));	// Avoid myself != NULL checks

/* Setup the pinfo structure for this process.  There may already be a
   _pinfo for this "pid" if h != NULL. */

void
pinfo::thisproc (HANDLE h)
{
  procinfo = NULL;

  DWORD flags = PID_IN_USE | PID_ACTIVE;
  if (!h)
    {
      h = INVALID_HANDLE_VALUE;
      cygheap->pid = cygwin_pid (myself_initial.pid);
      flags |= PID_NEW;
    }

  init (cygheap->pid, flags, h);
  procinfo->process_state |= PID_IN_USE;
  procinfo->dwProcessId = myself_initial.pid;
  procinfo->sendsig = myself_initial.sendsig;
  wcscpy (procinfo->progname, myself_initial.progname);
  debug_printf ("myself dwProcessId %u", procinfo->dwProcessId);
  if (h != INVALID_HANDLE_VALUE)
    {
      /* here if execed */
      static pinfo NO_COPY myself_identity;
      myself_identity.init (cygwin_pid (procinfo->dwProcessId), PID_EXECED, NULL);
      procinfo->exec_sendsig = NULL;
      procinfo->exec_dwProcessId = 0;
      myself_identity->ppid = procinfo->pid;
    }
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

      myself.thisproc (NULL);
      myself->pgid = myself->sid = myself->pid;
      myself->ctty = -1;
      myself->uid = ILLEGAL_UID;
      myself->gid = ILLEGAL_GID;
      environ_init (NULL, 0);	/* call after myself has been set up */
      myself->nice = winprio_to_nice (GetPriorityClass (GetCurrentProcess ()));
      myself->ppid = 1;		/* always set last */
      debug_printf ("Set nice to %d", myself->nice);
    }

  myself->process_state |= PID_ACTIVE;
  myself->process_state &= ~(PID_INITIALIZING | PID_EXITED | PID_REAPED);
  myself.preserve ();
  debug_printf ("pid %d, pgid %d, process_state %y", myself->pid, myself->pgid, myself->process_state);
}

DWORD
pinfo::status_exit (DWORD x)
{
  switch (x)
    {
    case STATUS_DLL_NOT_FOUND:
      {
	path_conv pc;
	if (!procinfo)
	   pc.check ("/dev/null", PC_NOWARN | PC_POSIX);
	else
	  {
	    UNICODE_STRING uc;
	    RtlInitUnicodeString(&uc, procinfo->progname);
	    pc.check (&uc, PC_NOWARN | PC_POSIX);
	  }
	small_printf ("%s: error while loading shared libraries: %s: cannot "
		      "open shared object file: No such file or directory\n",
		      pc.get_posix (), find_first_notloaded_dll (pc));
	x = 127 << 8;
      }
      break;
    case STATUS_ILLEGAL_DLL_PSEUDO_RELOCATION: /* custom error value */
      /* We've already printed the error message in pseudo-reloc.c */
      x = 127 << 8;
      break;
    case STATUS_ACCESS_VIOLATION:
      x = SIGSEGV;
      break;
    case STATUS_ILLEGAL_INSTRUCTION:
      x = SIGILL;
      break;
    case STATUS_NO_MEMORY:
      /* If the PATH environment variable is longer than about 30K and the full
	 Windows environment is > 32K, startup of an exec'ed process fails with
	 STATUS_NO_MEMORY.  This happens with all Cygwin executables, as well
	 as, for instance, notepad, but it does not happen with CMD for some
	 reason (but note, the environment *in* CMD is broken and shortened).
	 This occurs at a point where there's no return to the exec'ing parent
	 process, so we have to find some way to inform the user what happened.
	 
	 FIXME: For now, just return with SIGBUS set.  Maybe it's better to add
	 a lengthy small_printf instead. */
      x = SIGBUS;
      break;
    default:
      debug_printf ("*** STATUS_%y\n", x);
      x = 127 << 8;
    }
  return EXITCODE_SET | x;
}

# define self (*this)
void
pinfo::set_exit_code (DWORD x)
{
  if (x >= 0xc0000000UL)
    self->exitcode = status_exit (x);
  else
    self->exitcode = EXITCODE_SET | (sigExeced ?: (x & 0xff) << 8);
}

void
pinfo::maybe_set_exit_code_from_windows ()
{
  DWORD x = 0xdeadbeef;
  DWORD oexitcode = self->exitcode;

  if (hProcess && !(self->exitcode & EXITCODE_SET))
    {
      WaitForSingleObject (hProcess, INFINITE);	/* just to be safe, in case
						   process hasn't quite exited
						   after closing pipe */
      GetExitCodeProcess (hProcess, &x);
      set_exit_code (x);
    }
  sigproc_printf ("pid %d, exit value - old %y, windows %y, cygwin %y",
		  self->pid, oexitcode, x, self->exitcode);
}

void
pinfo::exit (DWORD n)
{
  debug_only_printf ("winpid %d, exit %d", GetCurrentProcessId (), n);
  proc_terminate ();
  lock_process until_exit (true);
  cygthread::terminate ();

  if (n != EXITCODE_NOSET)
    self->exitcode = EXITCODE_SET | n;/* We're really exiting.  Record the UNIX exit code. */
  else
    maybe_set_exit_code_from_windows ();	/* may block */
  exit_state = ES_FINAL;

  if (myself->ctty > 0 && !iscons_dev (myself->ctty))
    {
      lock_ttys here;
      tty *t = cygwin_shared->tty[device::minor(myself->ctty)];
      if (!t->slave_alive ())
	t->setpgid (0);
    }

  /* FIXME:  There is a potential race between an execed process and its
     parent here.  I hated to add a mutex just for that, though.  */
  struct rusage r;
  fill_rusage (&r, GetCurrentProcess ());
  add_rusage (&self->rusage_self, &r);
  int exitcode = self->exitcode & 0xffff;
  if (!self->cygstarted)
    exitcode = ((exitcode & 0xff) << 8) | ((exitcode >> 8) & 0xff);
  sigproc_printf ("Calling ExitProcess n %y, exitcode %y", n, exitcode);
  if (!TerminateProcess (GetCurrentProcess (), exitcode))
    system_printf ("TerminateProcess failed, %E");
  ExitProcess (exitcode);
}
# undef self

inline void
pinfo::_pinfo_release ()
{
  if (procinfo)
    {
      void *unmap_procinfo = procinfo;
      procinfo = NULL;
      UnmapViewOfFile (unmap_procinfo);
    }
  HANDLE close_h;
  if (h)
    {
      close_h = h;
      h = NULL;
      ForceCloseHandle1 (close_h, pinfo_shared_handle);
    }
}

void
pinfo::init (pid_t n, DWORD flag, HANDLE h0)
{
  shared_locations shloc;
  h = NULL;
  if (myself && !(flag & PID_EXECED)
      && (n == myself->pid || (DWORD) n == myself->dwProcessId))
    {
      procinfo = myself;
      destroy = 0;
      return;
    }

  int createit = flag & (PID_IN_USE | PID_EXECED);
  DWORD access = FILE_MAP_READ
		 | (flag & (PID_IN_USE | PID_EXECED | PID_MAP_RW)
		    ? FILE_MAP_WRITE : 0);
  if (!h0 || myself.h)
    shloc = (flag & (PID_IN_USE | PID_EXECED)) ? SH_JUSTCREATE : SH_JUSTOPEN;
  else
    {
      shloc = SH_MYSELF;
      if (h0 == INVALID_HANDLE_VALUE)
	h0 = NULL;
    }

  procinfo = NULL;
  PSECURITY_ATTRIBUTES sa_buf = (PSECURITY_ATTRIBUTES) alloca (1024);
  PSECURITY_ATTRIBUTES sec_attribs = sec_user_nih (sa_buf, cygheap->user.sid(),
						   well_known_world_sid,
						   FILE_MAP_READ);

  for (int i = 0; i < 20; i++)
    {
      DWORD mapsize;
      if (flag & PID_EXECED)
	mapsize = PINFO_REDIR_SIZE;
      else
	mapsize = sizeof (_pinfo);

      procinfo = (_pinfo *) open_shared (L"cygpid", n, h0, mapsize, &shloc,
					 sec_attribs, access);
      if (!h0)
	{
	  if (createit)
	    __seterrno ();
	  return;
	}

      if (!procinfo)
	{
	  if (exit_state)
	    return;

	  if (GetLastError () == ERROR_INVALID_HANDLE)
	    api_fatal ("MapViewOfFileEx h0 %p, i %d failed, %E", h0, i);

	  debug_printf ("MapViewOfFileEx h0 %p, i %d failed, %E", h0, i);
	  yield ();
	  continue;
	}

      bool created = shloc != SH_JUSTOPEN;

      /* Detect situation where a transitional memory block is being retrieved.
	 If the block has been allocated with PINFO_REDIR_SIZE but not yet
	 updated with a PID_EXECED state then we'll retry.  */
      if (!created && !(flag & PID_NEW))
	/* If not populated, wait 2 seconds for procinfo to become populated.
	   Would like to wait with finer granularity but that is not easily
	   doable.  */
	for (int i = 0; i < 200 && !procinfo->ppid; i++)
	  Sleep (10);

      if (!created && createit && (procinfo->process_state & PID_REAPED))
	{
	  memset (procinfo, 0, sizeof (*procinfo));
	  created = true;	/* Lie that we created this - just reuse old
				   shared memory */
	}

      if ((procinfo->process_state & PID_REAPED)
	  || ((procinfo->process_state & PID_INITIALIZING) && (flag & PID_NOREDIR)
	      && cygwin_pid (procinfo->dwProcessId) != procinfo->pid))
	{
	  set_errno (ESRCH);
	  break;
	}

      if (procinfo->process_state & PID_EXECED)
	{
	  pid_t realpid = procinfo->pid;
	  debug_printf ("execed process windows pid %u, cygwin pid %d", n, realpid);
	  if (realpid == n)
	    api_fatal ("retrieval of execed process info for pid %d failed due to recursion.", n);

	  n = realpid;
	  CloseHandle (h0);
	  h0 = NULL;
	  goto loop;
	}

      /* In certain pathological cases, it is possible for the shared memory
	 region to exist for a while after a process has exited.  This should
	 only be a brief occurrence, so rather than introduce some kind of
	 locking mechanism, just loop.  */
      if (!created && createit && (procinfo->process_state & (PID_EXITED | PID_REAPED)))
	{
	  debug_printf ("looping because pid %d, procinfo->pid %d, "
			"procinfo->dwProcessid %u has PID_EXITED|PID_REAPED set",
			n, procinfo->pid, procinfo->dwProcessId);
	  goto loop;
	}

      if (flag & PID_NEW)
	procinfo->start_time = time (NULL);
      if (!created)
	/* nothing */;
      else if (!(flag & PID_EXECED))
	procinfo->pid = n;
      else
	{
	  procinfo->process_state |= PID_IN_USE | PID_EXECED;
	  procinfo->pid = myself->pid;
	}

      h = h0;	/* Success! */
      break;

    loop:
      _pinfo_release ();
      if (h0)
	yield ();
    }

  if (h)
    {
      destroy = 1;
      ProtectHandle1 (h, pinfo_shared_handle);
    }
  else
    {
      h = h0;
      _pinfo_release ();
    }
}

void
pinfo::set_acl()
{
  PACL acl_buf = (PACL) alloca (1024);
  SECURITY_DESCRIPTOR sd;
  NTSTATUS status;

  sec_acl (acl_buf, true, true, cygheap->user.sid (),
	   well_known_world_sid, FILE_MAP_READ);
  RtlCreateSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION);
  status = RtlSetDaclSecurityDescriptor (&sd, TRUE, acl_buf, FALSE);
  if (!NT_SUCCESS (status))
    debug_printf ("RtlSetDaclSecurityDescriptor %y", status);
  else if ((status = NtSetSecurityObject (h, DACL_SECURITY_INFORMATION, &sd)))
    debug_printf ("NtSetSecurityObject %y", status);
}

pinfo::pinfo (HANDLE parent, pinfo_minimal& from, pid_t pid):
  pinfo_minimal (), destroy (false), procinfo (NULL), waiter_ready (false),
  wait_thread (NULL)
{
  HANDLE herr;
  const char *duperr = NULL;
  if (!DuplicateHandle (parent, herr = from.rd_proc_pipe, GetCurrentProcess (),
			&rd_proc_pipe, 0, false, DUPLICATE_SAME_ACCESS))
    duperr = "couldn't duplicate parent rd_proc_pipe handle %p for forked child %d after exec, %E";
  else if (!DuplicateHandle (parent, herr = from.hProcess, GetCurrentProcess (),
			     &hProcess, 0, false, DUPLICATE_SAME_ACCESS))
    duperr = "couldn't duplicate parent process handle %p for forked child %d after exec, %E";
  else
    {
      h = NULL;
      DuplicateHandle (parent, from.h, GetCurrentProcess (), &h, 0, false,
		       DUPLICATE_SAME_ACCESS);
      init (pid, PID_MAP_RW, h);
      if (*this)
	return;
    }

  if (duperr)
    debug_printf (duperr, herr, pid);

  /* Returning with procinfo == NULL.  Any open handles will be closed by the
     destructor. */
}

const char *
_pinfo::_ctty (char *buf)
{
  if (ctty <= 0)
    strcpy (buf, "no ctty");
  else
    {
      device d;
      d.parse (ctty);
      __small_sprintf (buf, "ctty %s", d.name ());
    }
  return buf;
}

bool
_pinfo::set_ctty (fhandler_termios *fh, int flags)
{
  tty_min& tc = *fh->tc ();
  debug_printf ("old %s, ctty device number %y, tc.ntty device number %y flags & O_NOCTTY %y", __ctty (), ctty, tc.ntty, flags & O_NOCTTY);
  if (fh && (ctty <= 0 || ctty == tc.ntty) && !(flags & O_NOCTTY))
    {
      ctty = tc.ntty;
      if (cygheap->ctty != fh->archetype)
	{
	  debug_printf ("cygheap->ctty %p, archetype %p", cygheap->ctty, fh->archetype);
	  if (!cygheap->ctty)
	    syscall_printf ("ctty was NULL");
	  else
	    {
	      syscall_printf ("ctty %p, usecount %d", cygheap->ctty,
			      cygheap->ctty->archetype_usecount (0));
	      cygheap->ctty->close ();
	    }
	  cygheap->ctty = (fhandler_termios *) fh->archetype;
	  if (cygheap->ctty)
	    {
	      fh->archetype_usecount (1);
	      /* guard ctty fh */
	      report_tty_counts (cygheap->ctty, "ctty", "");
	    }
	}

      lock_ttys here;
      syscall_printf ("attaching %s sid %d, pid %d, pgid %d, tty->pgid %d, tty->sid %d",
		      __ctty (), sid, pid, pgid, tc.getpgid (), tc.getsid ());
      if (!cygwin_finished_initializing && !myself->cygstarted
	  && pgid == pid && tc.getpgid () && tc.getsid ())
	pgid = tc.getpgid ();

      /* May actually need to do this:

	 if (sid == pid && !tc.getsid () || !procinfo (tc.getsid ())->exists)

	 but testing for process existence is expensive so we avoid it until
	 an obvious bug surfaces. */
      if (sid == pid && !tc.getsid ())
	tc.setsid (sid);
      sid = tc.getsid ();
      /* See above */
      if (!tc.getpgid () && pgid == pid)
	tc.setpgid (pgid);
    }
  debug_printf ("cygheap->ctty now %p, archetype %p", cygheap->ctty, fh ? fh->archetype : NULL);
  return ctty > 0;
}

/* Test to determine if a process really exists and is processing signals.
 */
bool __reg1
_pinfo::exists ()
{
  return this && process_state && !(process_state & (PID_EXITED | PID_REAPED | PID_EXECED));
}

bool
_pinfo::alive ()
{
  HANDLE h = OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION, false,
			  dwProcessId);
  if (h)
    CloseHandle (h);
  return !!h;
}

DWORD WINAPI
commune_process (void *arg)
{
  siginfo_t& si = *((siginfo_t *) arg);
  tmp_pathbuf tp;
  char *path = tp.c_get ();
  DWORD nr;
  HANDLE& tothem = si._si_commune._si_write_handle;
  HANDLE process_sync =
    OpenSemaphore (SYNCHRONIZE, false, shared_name (path, "commune", si.si_pid));
  if (process_sync)		// FIXME: this test shouldn't be necessary
    ProtectHandle (process_sync);

  lock_process now;
  if (si._si_commune._si_code & PICOM_EXTRASTR)
    si._si_commune._si_str = (char *) (&si + 1);

  switch (si._si_commune._si_code)
    {
    case PICOM_CMDLINE:
      {
	sigproc_printf ("processing PICOM_CMDLINE");
	unsigned n = 0;
	const char *argv[__argc_safe + 1];

	for (int i = 0; i < __argc_safe; i++)
	  {
	    if (IsBadStringPtr (__argv[i], INT32_MAX))
	      argv[i] = "";
	    else
	      argv[i] = __argv[i];
	    n += strlen (argv[i]) + 1;
	  }
	argv[__argc_safe] = NULL;
	if (!WritePipeOverlapped (tothem, &n, sizeof n, &nr, 1000L))
	  {
	    /*__seterrno ();*/	// this is run from the signal thread, so don't set errno
	    sigproc_printf ("WritePipeOverlapped sizeof argv failed, %E");
	  }
	else
	  for (const char **a = argv; *a; a++)
	    if (!WritePipeOverlapped (tothem, *a, strlen (*a) + 1, &nr, 1000L))
	      {
		sigproc_printf ("WritePipeOverlapped arg %d failed, %E",
				a - argv);
		break;
	      }
	break;
      }
    case PICOM_CWD:
      {
	sigproc_printf ("processing PICOM_CWD");
	unsigned int n = strlen (cygheap->cwd.get (path, 1, 1, NT_MAX_PATH)) + 1;
	if (!WritePipeOverlapped (tothem, &n, sizeof n, &nr, 1000L))
	  sigproc_printf ("WritePipeOverlapped sizeof cwd failed, %E");
	else if (!WritePipeOverlapped (tothem, path, n, &nr, 1000L))
	  sigproc_printf ("WritePipeOverlapped cwd failed, %E");
	break;
      }
    case PICOM_ROOT:
      {
	sigproc_printf ("processing PICOM_ROOT");
	unsigned n;
	if (cygheap->root.exists ())
	  n = strlen (strcpy (path, cygheap->root.posix_path ())) + 1;
	else
	  n = strlen (strcpy (path, "/")) + 1;
	if (!WritePipeOverlapped (tothem, &n, sizeof n, &nr, 1000L))
	  sigproc_printf ("WritePipeOverlapped sizeof root failed, %E");
	else if (!WritePipeOverlapped (tothem, path, n, &nr, 1000L))
	  sigproc_printf ("WritePipeOverlapped root failed, %E");
	break;
      }
    case PICOM_FDS:
      {
	sigproc_printf ("processing PICOM_FDS");
	unsigned int n = 0;
	int fd;
	cygheap_fdenum cfd;
	while ((fd = cfd.next ()) >= 0)
	  n += sizeof (int);
	cfd.rewind ();
	if (!WritePipeOverlapped (tothem, &n, sizeof n, &nr, 1000L))
	  sigproc_printf ("WritePipeOverlapped sizeof fds failed, %E");
	else
	  while ((fd = cfd.next ()) >= 0)
	    if (!WritePipeOverlapped (tothem, &fd, sizeof fd, &nr, 1000L))
	      {
		sigproc_printf ("WritePipeOverlapped fd %d failed, %E", fd);
		break;
	      }
	break;
      }
    case PICOM_PIPE_FHANDLER:
      {
	sigproc_printf ("processing PICOM_FDS");
	int64_t unique_id = si._si_commune._si_pipe_unique_id;
	unsigned int n = 0;
	cygheap_fdenum cfd;
	while (cfd.next () >= 0)
	  if (cfd->get_unique_id () == unique_id)
	    {
	      fhandler_pipe *fh = cfd;
	      n = sizeof *fh;
	      if (!WritePipeOverlapped (tothem, &n, sizeof n, &nr, 1000L))
		sigproc_printf ("WritePipeOverlapped sizeof hdl failed, %E");
	      else if (!WritePipeOverlapped (tothem, fh, n, &nr, 1000L))
		sigproc_printf ("WritePipeOverlapped hdl failed, %E");
	      break;
	    }
	if (!n && !WritePipeOverlapped (tothem, &n, sizeof n, &nr, 1000L))
	  sigproc_printf ("WritePipeOverlapped sizeof hdl failed, %E");
	break;
      }
    case PICOM_FD:
      {
	sigproc_printf ("processing PICOM_FD");
	int fd = si._si_commune._si_fd;
	unsigned int n = 0;
	cygheap_fdget cfd (fd);
	if (cfd < 0)
	  n = strlen (strcpy (path, "")) + 1;
	else
	  n = strlen (cfd->get_proc_fd_name (path)) + 1;
	if (!WritePipeOverlapped (tothem, &n, sizeof n, &nr, 1000L))
	  sigproc_printf ("WritePipeOverlapped sizeof fd failed, %E");
	else if (!WritePipeOverlapped (tothem, path, n, &nr, 1000L))
	  sigproc_printf ("WritePipeOverlapped fd failed, %E");
	break;
      }
    }
  if (process_sync)
    {
      DWORD res = WaitForSingleObject (process_sync, 5000);
      if (res != WAIT_OBJECT_0)
	sigproc_printf ("WFSO failed - %u, %E", res);
      else
	sigproc_printf ("synchronized with pid %d", si.si_pid);
      ForceCloseHandle (process_sync);
    }
  CloseHandle (tothem);
  _my_tls._ctinfo->auto_release ();
  return 0;
}

commune_result
_pinfo::commune_request (__uint32_t code, ...)
{
  DWORD nr;
  commune_result res;
  va_list args;
  siginfo_t si = {0};
  HANDLE& hp = si._si_commune._si_process_handle;
  HANDLE& fromthem = si._si_commune._si_read_handle;
  HANDLE request_sync = NULL;

  res.s = NULL;
  res.n = 0;

  if (!this || !pid)
    {
      set_errno (ESRCH);
      goto err;
    }
  if (ISSTATE (this, PID_NOTCYGWIN))
    {
      set_errno (ENOTSUP);
      goto err;
    }

  va_start (args, code);
  si._si_commune._si_code = code;
  switch (code)
    {
    case PICOM_PIPE_FHANDLER:
      si._si_commune._si_pipe_unique_id = va_arg (args, int64_t);
      break;

    case PICOM_FD:
      si._si_commune._si_fd = va_arg (args, int);
      break;

    break;
    }
  va_end (args);

  char name_buf[MAX_PATH];
  request_sync = CreateSemaphore (&sec_none_nih, 0, INT32_MAX,
				  shared_name (name_buf, "commune", myself->pid));
  if (!request_sync)
    goto err;
  ProtectHandle (request_sync);

  si.si_signo = __SIGCOMMUNE;
  if (sig_send (this, si))
    {
      ForceCloseHandle (request_sync);	/* don't signal semaphore since there was apparently no receiving process */
      request_sync = NULL;
      goto err;
    }

  DWORD n;
  switch (code)
    {
    case PICOM_CMDLINE:
    case PICOM_CWD:
    case PICOM_ROOT:
    case PICOM_FDS:
    case PICOM_FD:
    case PICOM_PIPE_FHANDLER:
      if (!ReadPipeOverlapped (fromthem, &n, sizeof n, &nr, 1000L)
	  || nr != sizeof n)
	{
	  __seterrno ();
	  goto err;
	}
      if (!n)
	res.s = NULL;
      else
	{
	  res.s = (char *) cmalloc_abort (HEAP_COMMUNE, n);
	  char *p;
	  for (p = res.s;
	       n && ReadPipeOverlapped (fromthem, p, n, &nr, 1000L);
	       p += nr, n -= nr)
	    continue;
	  if (n)
	    {
	      __seterrno ();
	      goto err;
	    }
	  res.n = p - res.s;
	}
      break;
    }
  goto out;

err:
  memset (&res, 0, sizeof (res));

out:
  if (request_sync)
    {
      LONG res;
      ReleaseSemaphore (request_sync, 1, &res);
      ForceCloseHandle (request_sync);
    }
  if (hp)
    CloseHandle (hp);
  if (fromthem)
    CloseHandle (fromthem);
  return res;
}

fhandler_pipe *
_pinfo::pipe_fhandler (int64_t unique_id, size_t &n)
{
  if (!this || !pid)
    return NULL;
  if (pid == myself->pid)
    return NULL;
  commune_result cr = commune_request (PICOM_PIPE_FHANDLER, unique_id);
  n = cr.n;
  return (fhandler_pipe *) cr.s;
}

char *
_pinfo::fd (int fd, size_t &n)
{
  char *s;
  if (!this || !pid)
    return NULL;
  if (pid != myself->pid)
    {
      commune_result cr = commune_request (PICOM_FD, fd);
      s = cr.s;
      n = cr.n;
    }
  else
    {
      cygheap_fdget cfd (fd);
      if (cfd < 0)
	s = cstrdup ("");
      else
	s = cfd->get_proc_fd_name ((char *) cmalloc_abort (HEAP_COMMUNE, NT_MAX_PATH));
      n = strlen (s) + 1;
    }
  return s;
}

char *
_pinfo::fds (size_t &n)
{
  char *s;
  if (!this || !pid)
    return NULL;
  if (pid != myself->pid)
    {
      commune_result cr = commune_request (PICOM_FDS);
      s = cr.s;
      n = cr.n;
    }
  else
    {
      n = 0;
      int fd;
      cygheap_fdenum cfd (true);
      while ((fd = cfd.next ()) >= 0)
	n += sizeof (int);
      cfd.rewind ();
      s = (char *) cmalloc_abort (HEAP_COMMUNE, n);
      int *p = (int *) s;
      while ((fd = cfd.next ()) >= 0 && (char *) p - s < (int) n)
	*p++ = fd;
    }
  return s;
}

char *
_pinfo::root (size_t& n)
{
  char *s;
  if (!this || !pid)
    return NULL;
  if (pid != myself->pid && !ISSTATE (this, PID_NOTCYGWIN))
    {
      commune_result cr = commune_request (PICOM_ROOT);
      s = cr.s;
      n = cr.n;
    }
  else
    {
      if (cygheap->root.exists ())
	s = cstrdup (cygheap->root.posix_path ());
      else
	s = cstrdup ("/");
      n = strlen (s) + 1;
    }
  return s;
}

static HANDLE
open_commune_proc_parms (DWORD pid, PRTL_USER_PROCESS_PARAMETERS prupp)
{
  HANDLE proc;
  NTSTATUS status;
  PROCESS_BASIC_INFORMATION pbi;
  PEB lpeb;

  proc = OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
		      FALSE, pid);
  if (!proc)
    return NULL;
  status = NtQueryInformationProcess (proc, ProcessBasicInformation,
				      &pbi, sizeof pbi, NULL);
  if (NT_SUCCESS (status)
      && ReadProcessMemory (proc, pbi.PebBaseAddress, &lpeb, sizeof lpeb, NULL)
      && ReadProcessMemory (proc, lpeb.ProcessParameters, prupp, sizeof *prupp,
			    NULL))
	return proc;
  NtClose (proc);
  return NULL;
}

char *
_pinfo::cwd (size_t& n)
{
  char *s = NULL;
  if (!this || !pid)
    return NULL;
  if (ISSTATE (this, PID_NOTCYGWIN))
    {
      RTL_USER_PROCESS_PARAMETERS rupp;
      HANDLE proc = open_commune_proc_parms (dwProcessId, &rupp);

      n = 0;
      if (!proc)
	return NULL;

      tmp_pathbuf tp;
      PWCHAR cwd = tp.w_get ();

      if (ReadProcessMemory (proc, rupp.CurrentDirectoryName.Buffer,
			     cwd, rupp.CurrentDirectoryName.Length,
			     NULL))
	{
	  /* Drop trailing backslash, add trailing \0 in passing. */
	  cwd[rupp.CurrentDirectoryName.Length / sizeof (WCHAR) - 1]
	  = L'\0';
	  s = (char *) cmalloc_abort (HEAP_COMMUNE, NT_MAX_PATH);
	  mount_table->conv_to_posix_path (cwd, s, 0);
	  n = strlen (s) + 1;
	}
      NtClose (proc);
    }
  else if (pid != myself->pid)
    {
      commune_result cr = commune_request (PICOM_CWD);
      s = cr.s;
      n = cr.n;
    }
  else
    {
      s = (char *) cmalloc_abort (HEAP_COMMUNE, NT_MAX_PATH);
      cygheap->cwd.get (s, 1, 1, NT_MAX_PATH);
      n = strlen (s) + 1;
    }
  return s;
}

char *
_pinfo::cmdline (size_t& n)
{
  char *s = NULL;
  if (!this || !pid)
    return NULL;
  if (ISSTATE (this, PID_NOTCYGWIN))
    {
      RTL_USER_PROCESS_PARAMETERS rupp;
      HANDLE proc = open_commune_proc_parms (dwProcessId, &rupp);

      n = 0;
      if (!proc)
	return NULL;

      tmp_pathbuf tp;
      PWCHAR cmdline = tp.w_get ();

      if (ReadProcessMemory (proc, rupp.CommandLine.Buffer, cmdline,
			     rupp.CommandLine.Length, NULL))
	{
	  /* Add trailing \0. */
	  cmdline[rupp.CommandLine.Length / sizeof (WCHAR)]
	  = L'\0';
	  n = sys_wcstombs_alloc (&s, HEAP_COMMUNE, cmdline,
				  rupp.CommandLine.Length
				  / sizeof (WCHAR));
	  /* Quotes & Spaces post-processing. */
	  bool in_quote = false;
	  for (char *c = s; *c; ++c)
	    if (*c == '"')
	      in_quote = !in_quote;
	    else if (*c == ' ' && !in_quote)
	     *c = '\0';
	}
      NtClose (proc);
    }
  else if (pid != myself->pid)
    {
      commune_result cr = commune_request (PICOM_CMDLINE);
      s = cr.s;
      n = cr.n;
    }
  else
    {
      n = 0;
      for (char **a = __argv; *a; a++)
	n += strlen (*a) + 1;
      char *p;
      p = s = (char *) cmalloc_abort (HEAP_COMMUNE, n);
      for (char **a = __argv; *a; a++)
	{
	  strcpy (p, *a);
	  p = strchr (p, '\0') + 1;
	}
    }
  return s;
}

/* This is the workhorse which waits for the write end of the pipe
   created during new process creation.  If the pipe is closed or a zero
   is received on the pipe, it is assumed that the cygwin pid has exited.
   Otherwise, various "signals" can be sent to the parent to inform the
   parent to perform a certain action. */
static DWORD WINAPI
proc_waiter (void *arg)
{
  pinfo vchild = *(pinfo *) arg;
  ((pinfo *) arg)->waiter_ready = true;

  siginfo_t si = {0};
  si.si_signo = SIGCHLD;
  si.si_code = CLD_EXITED;
  si.si_pid = vchild->pid;
#if 0	// FIXME: This is tricky to get right
  si.si_utime = pchildren[rc]->rusage_self.ru_utime;
  si.si_stime = pchildren[rc].rusage_self.ru_stime;
#endif
  pid_t pid = vchild->pid;
  bool its_me = vchild == myself;

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

      if (!its_me && have_execed_cygwin)
	break;

      si.si_uid = vchild->uid;

      switch (buf)
	{
	case __ALERT_ALIVE:
	  continue;
	case 0:
	  /* Child exited.  Do some cleanup and signal myself.  */
	  vchild.maybe_set_exit_code_from_windows ();
	  if (WIFEXITED (vchild->exitcode))
	    si.si_code = CLD_EXITED;
	  else if (WCOREDUMP (vchild->exitcode))
	    si.si_code = CLD_DUMPED;
	  else
	    si.si_code = CLD_KILLED;
	  si.si_status = vchild->exitcode;
	  vchild->process_state = PID_EXITED;
	  /* This should always be last.  Do not use vchild-> beyond this point */
	  break;
	case SIGTTIN:
	case SIGTTOU:
	case SIGTSTP:
	case SIGSTOP:
	  if (ISSTATE (myself, PID_NOCLDSTOP))	// FIXME: No need for this flag to be in _pinfo any longer
	    continue;
	  /* Child stopped.  Signal myself.  */
	  si.si_code = CLD_STOPPED;
	  break;
	case SIGCONT:
	  continue;
	default:
	  system_printf ("unknown value %d on proc pipe", buf);
	  continue;
	}

      if (its_me && ch_spawn.signal_myself_exited ())
	break;

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
  vchild.wait_thread = NULL;
  _my_tls._ctinfo->auto_release ();	/* automatically return the cygthread to the cygthread pool */
  return 0;
}

/* function to set up the process pipe and kick off proc_waiter */
bool
pinfo::wait ()
{
  preserve ();		/* Preserve the shared memory associated with the pinfo */

  waiter_ready = false;
  /* Fire up a new thread to track the subprocess */
  cygthread *h = new cygthread (proc_waiter, this, "waitproc");
  if (!h)
    sigproc_printf ("tracking thread creation failed for pid %d", (*this)->pid);
  else
    {
      wait_thread = h;
      sigproc_printf ("created tracking thread for pid %d, winpid %y, rd_proc_pipe %p",
		      (*this)->pid, (*this)->dwProcessId, rd_proc_pipe);
    }

  return true;
}

/* function to send a "signal" to the parent when something interesting happens
   in the child. */
bool
_pinfo::alert_parent (char sig)
{
  DWORD nb = 0;

  /* Send something to our parent.  If the parent has gone away, close the pipe.
     Don't send if this is an exec stub.

     FIXME: Is there a race here if we run this while another thread is attempting
     to exec()? */
  if (my_wr_proc_pipe)
    {
      if (WriteFile (my_wr_proc_pipe, &sig, 1, &nb, NULL))
	/* all is well */;
      else if (GetLastError () != ERROR_BROKEN_PIPE)
	debug_printf ("sending %d notification to parent failed, %E", sig);
      else
	{
	  ppid = 1;
	  HANDLE closeit = my_wr_proc_pipe;
	  my_wr_proc_pipe = NULL;
	  ForceCloseHandle1 (closeit, wr_proc_pipe);
	}
    }
  return (bool) nb;
}

void
pinfo::release ()
{
  _pinfo_release ();
  HANDLE close_h;
  if (rd_proc_pipe)
    {
      close_h = rd_proc_pipe;
      rd_proc_pipe = NULL;
      ForceCloseHandle1 (close_h, rd_proc_pipe);
    }
  if (hProcess)
    {
      close_h = hProcess;
      hProcess = NULL;
      ForceCloseHandle1 (close_h, childhProc);
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


#define slop_pidlist 200
#define size_pidlist(i) (sizeof (pidlist[0]) * ((i) + 1))
#define size_pinfolist(i) (sizeof (pinfolist[0]) * ((i) + 1))
class _onreturn
{
  HANDLE h;
public:
  ~_onreturn ()
  {
    if (h)
      {
	CloseHandle (h);
      }
  }
  void no_close_handle (pinfo& p)
  {
    p.hProcess = h;
    h = NULL;
  }
  _onreturn (): h (NULL) {}
  void operator = (HANDLE h0) {h = h0;}
  operator HANDLE () const {return h;}
};

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

  _onreturn onreturn;
  pinfo& p = pinfolist[nelem];
  memset (&p, 0, sizeof (p));

  bool perform_copy;
  if (cygpid == myself->pid)
    {
      p = myself;
      perform_copy = false;
    }
  else
    {
      /* Open a process to prevent a subsequent exit from invalidating the
	 shared memory region. */
      onreturn = OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION, false, pid);

      /* If we couldn't open the process then we don't have rights to it and should
	 make a copy of the shared memory area when it exists (it may not).  */
      perform_copy = onreturn ? make_copy : true;

      p.init (cygpid, PID_NOREDIR | pinfo_access, NULL);
    }

  /* If we're just looking for winpids then don't do any special cygwin "stuff* */
  if (winpid)
    {
      perform_copy = true;
      goto out;
    }

  /* !p means that we couldn't find shared memory for this pid.  Probably means
     that it isn't a cygwin process. */
  if (!p)
    {
      if (!pinfo_access)
	return;
      p.init (cygpid, PID_NOREDIR, NULL);
      if (!p)
	return;
    }

  /* Scan list of previously recorded pids to make sure that this pid hasn't
     shown up before.  This can happen when a process execs. */
  for (unsigned i = 0; i < nelem; i++)
    if (pinfolist[i]->pid == p->pid)
      {
	if ((_pinfo *) p != (_pinfo *) myself)
	  p.release ();
	return;
      }

out:
  /* Exit here.

     If p is "false" then, eventually any opened process handle will be closed and
     the function will exit without adding anything to the pid list.

     If p is "true" then we've discovered a cygwin process.

     Handle "myself" differently.  Don't copy it and close/zero the handle we
     just opened to it.
     If not performing a copy, then keep the process handle open for the duration
     of the life of the procinfo region to potential races when a new process uses
     this pid.
     Otherwise, malloc some memory for a copy of the shared memory.

     If the malloc failed, then "oh well".  Just keep the shared memory around
     and eventually close the handle when the winpids goes out of scope.

     If malloc succeeds, copy the procinfo we just grabbed into the new region,
     release the shared memory and allow the handle to be closed when this
     function returns.

     Oh, and add the pid to the list and bump the number of elements.  */

  if (p)
    {
      if (p == (_pinfo *) myself)
	/* handle specially.  Close the handle but (eventually) don't
	   deallocate procinfo in release call */;
      else if (!perform_copy)
	onreturn.no_close_handle (p);	/* Don't close the handle until release */
      else
	{
	  _pinfo *pnew = (_pinfo *) malloc (sizeof (*p.procinfo));
	  if (!pnew)
	    onreturn.no_close_handle (p);
	  else
	    {
	      *pnew = *p.procinfo;
	      p.release ();
	      p.procinfo = pnew;
	      p.destroy = false;
	      if (winpid)
		p->dwProcessId = pid;
	    }
	}
    }
  if (p || winpid)
    pidlist[nelem++] = !p ? pid : p->dwProcessId;
}

DWORD
winpids::enum_processes (bool winpid)
{
  DWORD nelem = 0;

  if (!winpid)
    {
      HANDLE dir = get_shared_parent_dir ();
      BOOLEAN restart = TRUE;
      ULONG context;
      struct fdbi
	{
	  DIRECTORY_BASIC_INFORMATION dbi;
	  WCHAR buf[2][NAME_MAX + 1];
	} f;
      while (NT_SUCCESS (NtQueryDirectoryObject (dir, &f, sizeof f, TRUE,
						 restart, &context, NULL)))
	{
	  restart = FALSE;
	  f.dbi.ObjectName.Buffer[f.dbi.ObjectName.Length / sizeof (WCHAR)] = L'\0';
	  if (wcsncmp (f.dbi.ObjectName.Buffer, L"cygpid.", 7) == 0)
	    {
	    DWORD pid = wcstoul (f.dbi.ObjectName.Buffer + 7, NULL, 10);
	    add (nelem, false, pid);
	  }
	}
    }
  else
    {
      static DWORD szprocs;
      static PSYSTEM_PROCESS_INFORMATION procs;

      while (1)
	{
	  PSYSTEM_PROCESS_INFORMATION new_p = (PSYSTEM_PROCESS_INFORMATION)
	    realloc (procs, szprocs += 200 * sizeof (*procs));
	  if (!new_p)
	    {
	      system_printf ("out of memory reading system process "
			     "information");
	      return 0;
	    }
	  procs = new_p;
	  NTSTATUS status = NtQuerySystemInformation (SystemProcessInformation,
						      procs, szprocs, NULL);
	  if (NT_SUCCESS (status))
	    break;

	  if (status != STATUS_INFO_LENGTH_MISMATCH)
	    {
	      system_printf ("error %y reading system process information",
			     status);
	      return 0;
	    }
	}

      PSYSTEM_PROCESS_INFORMATION px = procs;
      while (1)
	{
	  if (px->UniqueProcessId)
	    add (nelem, true, (DWORD) (uintptr_t) px->UniqueProcessId);
	  if (!px->NextEntryOffset)
	    break;
          px = (PSYSTEM_PROCESS_INFORMATION) ((char *) px + px->NextEntryOffset);
	}
    }
  return nelem;
}

void
winpids::set (bool winpid)
{
  npids = enum_processes (winpid);
  if (pidlist)
    pidlist[npids] = 0;
}

DWORD
winpids::enum_init (bool winpid)
{
  return enum_processes (winpid);
}

void
winpids::release ()
{
  _pinfo *p;
  for (unsigned i = 0; i < npids; i++)
    if (pinfolist[i] == (_pinfo *) myself)
      continue;
    else if (pinfolist[i].hProcess)
      pinfolist[i].release ();
    else if ((p = pinfolist[i]))
      {
	pinfolist[i].procinfo = NULL;
	free (p);
      }
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
