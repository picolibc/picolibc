/* fork.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "cygerrno.h"
#include "sigproc.h"
#include "pinfo.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info.h"
#include "cygtls.h"
#include "tls_pbuf.h"
#include "shared_info.h"
#include "dll_init.h"
#include "cygmalloc.h"
#include "ntdll.h"

#define NPIDS_HELD 4

/* Timeout to wait for child to start, parent to init child, etc.  */
/* FIXME: Once things stabilize, bump up to a few minutes.  */
#define FORK_WAIT_TIMEOUT (300 * 1000)     /* 300 seconds */

static int dofork (bool *with_forkables);
class frok
{
  frok (bool *forkables)
    : with_forkables (forkables)
  {}
  bool *with_forkables;
  bool load_dlls;
  child_info_fork ch;
  const char *errmsg;
  int child_pid;
  int this_errno;
  HANDLE hchild;
  int __stdcall parent (volatile char * volatile here);
  int __stdcall child (volatile char * volatile here);
  bool error (const char *fmt, ...);
  friend int dofork (bool *with_forkables);
};

static void
resume_child (HANDLE forker_finished)
{
  SetEvent (forker_finished);
  debug_printf ("signalled child");
  return;
}

/* Notify parent that it is time for the next step. */
static void __stdcall
sync_with_parent (const char *s, bool hang_self)
{
  debug_printf ("signalling parent: %s", s);
  fork_info->ready (false);
  if (hang_self)
    {
      HANDLE h = fork_info->forker_finished;
      /* Wait for the parent to fill in our stack and heap.
	 Don't wait forever here.  If our parent dies we don't want to clog
	 the system.  If the wait fails, we really can't continue so exit.  */
      DWORD psync_rc = WaitForSingleObject (h, FORK_WAIT_TIMEOUT);
      debug_printf ("awake");
      switch (psync_rc)
	{
	case WAIT_TIMEOUT:
	  api_fatal ("WFSO timed out %s", s);
	  break;
	case WAIT_FAILED:
	  if (GetLastError () == ERROR_INVALID_HANDLE &&
	      WaitForSingleObject (fork_info->forker_finished, 1) != WAIT_FAILED)
	    break;
	  api_fatal ("WFSO failed %s, fork_finished %p, %E", s,
		     fork_info->forker_finished);
	  break;
	default:
	  debug_printf ("no problems");
	  break;
	}
    }
}

bool
frok::error (const char *fmt, ...)
{
  DWORD exit_code = ch.exit_code;
  if (!exit_code && hchild)
    {
      exit_code = ch.proc_retry (hchild);
      if (!exit_code)
	return false;
    }
  if (exit_code != EXITCODE_FORK_FAILED)
    {
      va_list ap;
      static char buf[NT_MAX_PATH + 256];
      va_start (ap, fmt);
      __small_vsprintf (buf, fmt, ap);
      errmsg = buf;
    }
  return true;
}

/* Set up a pipe which will track the life of a "pid" through
   even after we've exec'ed.  */
void
child_info::prefork (bool detached)
{
  if (!detached)
    {
      if (!CreatePipe (&rd_proc_pipe, &wr_proc_pipe, &sec_none_nih, 16))
	api_fatal ("prefork: couldn't create pipe process tracker, %E");

      if (!SetHandleInformation (wr_proc_pipe, HANDLE_FLAG_INHERIT,
				 HANDLE_FLAG_INHERIT))
	api_fatal ("prefork: couldn't set process pipe(%p) inherit state, %E",
		   wr_proc_pipe);
      ProtectHandle1 (rd_proc_pipe, rd_proc_pipe);
      ProtectHandle1 (wr_proc_pipe, wr_proc_pipe);
    }
}

int __stdcall
frok::child (volatile char * volatile here)
{
  cygheap_fdenum cfd (false);
  while (cfd.next () >= 0)
    if (cfd->get_major () == DEV_PTYM_MAJOR)
      {
	fhandler_base *fh = cfd;
	fhandler_pty_master *ptym = (fhandler_pty_master *) fh;
	if (ptym->get_pseudo_console ())
	  {
	    debug_printf ("found a PTY master %d: helper_PID=%d",
			  ptym->get_minor (), ptym->get_helper_process_id ());
	    if (fhandler_console::get_console_process_id (
				ptym->get_helper_process_id (), true))
	      /* Already attached */
	      break;
	    else
	      {
		if (ptym->attach_pcon_in_fork ())
		  {
		    FreeConsole ();
		    if (!AttachConsole (ptym->get_helper_process_id ()))
		      /* Error */;
		    else
		      break;
		  }
	      }
	  }
      }

  HANDLE& hParent = ch.parent;

  sync_with_parent ("after longjmp", true);
  debug_printf ("child is running.  pid %d, ppid %d, stack here %p",
		myself->pid, myself->ppid, __builtin_frame_address (0));
  sigproc_printf ("hParent %p, load_dlls %d", hParent, load_dlls);

  /* Make sure threadinfo information is properly set up. */
  if (&_my_tls != _main_tls)
    {
      _main_tls = &_my_tls;
      _main_tls->init_thread (NULL, NULL);
    }

  set_cygwin_privileges (hProcToken);
  clear_procimptoken ();
  cygheap->user.reimpersonate ();

#ifdef DEBUGGING
  if (GetEnvironmentVariableA ("FORKDEBUG", NULL, 0))
    try_to_debug ();
  char buf[80];
  /* This is useful for debugging fork problems.  Use gdb to attach to
     the pid reported here. */
  if (GetEnvironmentVariableA ("CYGWIN_FORK_SLEEP", buf, sizeof (buf)))
    {
      small_printf ("Sleeping %d after fork, pid %u\n", atoi (buf), GetCurrentProcessId ());
      Sleep (atoi (buf));
    }
#endif

  /* Incredible but true:  If we use sockets and SYSV IPC shared memory,
     there's a good chance that a duplicated socket in the child occupies
     memory which is needed to duplicate shared memory from the parent
     process, if the shared memory hasn't been duplicated already.
     The same goes very likely for "normal" mmap shared memory, too, but
     with SYSV IPC it was the first time observed.  So, *never* fixup
     fdtab before fixing up shared memory. */
  if (fixup_shms_after_fork ())
    api_fatal ("recreate_shm areas after fork failed");

  /* load dynamic dlls, if any, re-track main-executable and cygwin1.dll */
  dlls.load_after_fork (hParent);

  cygheap->fdtab.fixup_after_fork (hParent);

  /* Signal that we have successfully initialized, so the parent can
     - transfer data/bss for dynamically loaded dlls (if any), or
     - terminate the current fork call even if the child is initialized. */
  sync_with_parent ("performed fork fixups and dynamic dll loading", true);

  ForceCloseHandle1 (fork_info->forker_finished, forker_finished);

  pthread::atforkchild ();
  cygbench ("fork-child");
  ld_preload ();
  fixup_hooks_after_fork ();
  _my_tls.fixup_after_fork ();
  /* Clear this or the destructor will close them.  In the case of
     rd_proc_pipe that would be an invalid handle.  In the case of
     wr_proc_pipe it would be == my_wr_proc_pipe.  Both would be bad. */
  ch.rd_proc_pipe = ch.wr_proc_pipe = NULL;
  CloseHandle (hParent);
  hParent = NULL;
  cygwin_finished_initializing = true;
  return 0;
}

int __stdcall
frok::parent (volatile char * volatile stack_here)
{
  HANDLE forker_finished;
  DWORD rc;
  child_pid = -1;
  this_errno = 0;
  bool fix_impersonation = false;
  pinfo child;

  int c_flags = GetPriorityClass (GetCurrentProcess ());
  debug_printf ("priority class %d", c_flags);
  /* Per MSDN, this must be specified even if lpEnvironment is set to NULL,
     otherwise UNICODE characters in the parent environment are not copied
     correctly to the child.  Omitting it may scramble %PATH% on non-English
     systems. */
  c_flags |= CREATE_UNICODE_ENVIRONMENT;

  errmsg = NULL;
  hchild = NULL;

  /* If we don't have a console, then don't create a console for the
     child either.  */
  HANDLE console_handle = CreateFile ("CONOUT$", GENERIC_WRITE,
				      FILE_SHARE_READ | FILE_SHARE_WRITE,
				      &sec_none_nih, OPEN_EXISTING,
				      FILE_ATTRIBUTE_NORMAL, NULL);

  if (console_handle != INVALID_HANDLE_VALUE)
    CloseHandle (console_handle);
  else
    c_flags |= DETACHED_PROCESS;

  /* Some file types (currently only sockets) need extra effort in the
     parent after CreateProcess and before copying the datastructures
     to the child. So we have to start the child in suspend state,
     unfortunately, to avoid a race condition. */
  if (cygheap->fdtab.need_fixup_before ())
    c_flags |= CREATE_SUSPENDED;

  /* Remember if we need to load dynamically linked dlls.
     We do this here so that this information will be available
     in the parent and, when the stack is copied, in the child. */
  load_dlls = dlls.reload_on_fork && dlls.loaded_dlls;

  forker_finished = CreateEvent (&sec_all, FALSE, FALSE, NULL);
  if (forker_finished == NULL)
    {
      this_errno = geterrno_from_win_error ();
      error ("unable to allocate forker_finished event");
      return -1;
    }

  ProtectHandleINH (forker_finished);

  ch.forker_finished = forker_finished;

  ch.stackbase = NtCurrentTeb ()->Tib.StackBase;
  ch.stackaddr = NtCurrentTeb ()->DeallocationStack;
  if (!ch.stackaddr)
    {
      /* If DeallocationStack is NULL, we're running on an application-provided
	 stack.  If so, the entire stack is committed anyway and StackLimit
	 points to the allocation address of the stack.  Mark in guardsize that
	 we must not set up guard pages. */
      ch.stackaddr = ch.stacklimit = NtCurrentTeb ()->Tib.StackLimit;
      ch.guardsize = (size_t) -1;
    }
  else
    {
      /* Otherwise we're running on a system-allocated stack.  Since stack_here
	 is the address of the stack pointer we start the child with anyway, we
	 can set ch.stacklimit to this value rounded down to page size.  The
	 child will not need the rest of the stack anyway.  Guardsize depends
	 on whether we're running on a pthread or not.  If pthread, we fetch
	 the guardpage size from the pthread attribs, otherwise we use the
	 system default. */
      ch.stacklimit = (void *) ((uintptr_t) stack_here & ~(wincap.page_size () - 1));
      ch.guardsize = (&_my_tls != _main_tls && _my_tls.tid)
		     ? _my_tls.tid->attr.guardsize
		     : wincap.def_guard_page_size ();
    }
  debug_printf ("stack - bottom %p, top %p, addr %p, guardsize %ly",
		ch.stackbase, ch.stacklimit, ch.stackaddr, ch.guardsize);

  PROCESS_INFORMATION pi;
  STARTUPINFOW si;

  memset (&si, 0, sizeof (si));
  si.cb = sizeof si;

  si.lpReserved2 = (LPBYTE) &ch;
  si.cbReserved2 = sizeof (ch);

  bool locked = __malloc_lock ();

  /* Remove impersonation */
  cygheap->user.deimpersonate ();
  fix_impersonation = true;
  ch.refresh_cygheap ();
  ch.prefork ();	/* set up process tracking pipes. */

  *with_forkables = dlls.setup_forkables (*with_forkables);

  ch.silentfail (!*with_forkables); /* fail silently without forkables */

  tmp_pathbuf tp;
  PSECURITY_ATTRIBUTES sa = (PSECURITY_ATTRIBUTES) tp.w_get ();
  if (!sec_user_nih (sa, cygheap->user.saved_sid (),
		     well_known_authenticated_users_sid,
		     PROCESS_QUERY_LIMITED_INFORMATION))
    sa = &sec_none_nih;

  while (1)
    {
      PCWCHAR forking_progname = NULL;
      if (dlls.main_executable)
        forking_progname = dll_list::buffered_shortname
			   (dlls.main_executable->forkedntname ());
      if (!forking_progname || !*forking_progname)
	forking_progname = myself->progname;

      syscall_printf ("CreateProcessW (%W, %W, 0, 0, 1, %y, 0, 0, %p, %p)",
		      forking_progname, myself->progname, c_flags, &si, &pi);

      hchild = NULL;
      /* cygwin1.dll may reuse the forking_progname buffer, even
	 in case of failure: don't reuse forking_progname later */
      rc = CreateProcessW (forking_progname,	/* image to run */
			   GetCommandLineW (),	/* Take same space for command
						   line as in parent to make
						   sure child stack is allocated
						   in the same memory location
						   as in parent. */
			   sa,
			   sa,
			   TRUE,		/* inherit handles */
			   c_flags,
			   NULL,		/* environ filled in later */
			   0,			/* use cwd */
			   &si,
			   &pi);

      if (rc)
	debug_printf ("forked pid %u", pi.dwProcessId);
      else
	{
	  this_errno = geterrno_from_win_error ();
	  error ("CreateProcessW failed for '%W'", myself->progname);
	  dlls.release_forkables ();
	  memset (&pi, 0, sizeof (pi));
	  goto cleanup;
	}

      if (cygheap->fdtab.need_fixup_before ())
	{
	  cygheap->fdtab.fixup_before_fork (pi.dwProcessId);
	  ResumeThread (pi.hThread);
	}

      CloseHandle (pi.hThread);
      hchild = pi.hProcess;

      dlls.release_forkables ();

      /* Protect the handle but name it similarly to the way it will
	 be called in subproc handling. */
      ProtectHandle1 (hchild, childhProc);

      strace.write_childpid (pi.dwProcessId);

      /* Wait for subproc to initialize itself. */
      if (!ch.sync (pi.dwProcessId, hchild, FORK_WAIT_TIMEOUT))
	{
	  if (!error ("forked process %u died unexpectedly, retry %d, exit code %y",
		      pi.dwProcessId, ch.retry, ch.exit_code))
	    continue;
	  this_errno = EAGAIN;
	  goto cleanup;
	}
      break;
    }

  /* Restore impersonation */
  cygheap->user.reimpersonate ();
  fix_impersonation = false;

  child_pid = cygwin_pid (pi.dwProcessId);
  child.init (child_pid, PID_IN_USE | PID_NEW, NULL);

  if (!child)
    {
      this_errno = get_errno () == ENOMEM ? ENOMEM : EAGAIN;
      syscall_printf ("pinfo failed");
      goto cleanup;
    }

  child->nice = myself->nice;

  /* Initialize things that are done later in dll_crt0_1 that aren't done
     for the forkee.  */
  wcscpy (child->progname, myself->progname);

  /* Fill in fields in the child's process table entry.  */
  child->dwProcessId = pi.dwProcessId;
  child.hProcess = hchild;
  ch.postfork (child);

  /* Hopefully, this will succeed.  The alternative to doing things this
     way is to reserve space prior to calling CreateProcess and then fill
     it in afterwards.  This requires more bookkeeping than I like, though,
     so we'll just do it the easy way.  So, terminate any child process if
     we can't actually record the pid in the internal table. */
  if (!child.remember (false))
    {
      this_errno = EAGAIN;
#ifdef DEBUGGING0
      error ("child remember failed");
#endif
      goto cleanup;
    }

  /* CHILD IS STOPPED */
  debug_printf ("child is alive (but stopped)");


  /* Initialize, in order: stack, dll data, dll bss.
     data, bss, heap were done earlier (in dcrt0.cc)
     Note: variables marked as NO_COPY will not be copied since they are
     placed in a protected segment.  */

  const void *impure_beg;
  const void *impure_end;
  const char *impure;
  if (&_my_tls == _main_tls)
    impure_beg = impure_end = impure = NULL;
  else
    {
      impure = "impure";
      impure_beg = _impure_ptr;
      impure_end = _impure_ptr + 1;
    }
  rc = child_copy (hchild, true, !*with_forkables,
		   "stack", stack_here, ch.stackbase,
		   impure, impure_beg, impure_end,
		   NULL);

  __malloc_unlock ();
  locked = false;
  if (!rc)
    {
      this_errno = get_errno ();
      error ("pid %u, exitval %p", pi.dwProcessId, ch.exit_code);
      goto cleanup;
    }

  /* Now fill data/bss of any DLLs that were linked into the program. */
  for (dll *d = dlls.istart (DLL_LINK); d; d = dlls.inext ())
    {
      debug_printf ("copying data/bss of a linked dll");
      if (!child_copy (hchild, true, !*with_forkables,
		       "linked dll data", d->p.data_start, d->p.data_end,
		       "linked dll bss", d->p.bss_start, d->p.bss_end,
		       NULL))
	{
	  this_errno = get_errno ();
	  error ("couldn't copy linked dll data/bss");
	  goto cleanup;
	}
    }

  /* Start the child up, and then wait for it to
     perform fork fixups and dynamic dll loading (if any). */
  resume_child (forker_finished);
  if (!ch.sync (child->pid, hchild, FORK_WAIT_TIMEOUT))
    {
      this_errno = EAGAIN;
      error ("died waiting for dll loading");
      goto cleanup;
    }

  /* If DLLs were loaded in the parent, then the child has reloaded all
     of them and is now waiting to have all of the individual data and
     bss sections filled in. */
  if (load_dlls)
    {
      /* CHILD IS STOPPED */
      /* write memory of reloaded dlls */
      for (dll *d = dlls.istart (DLL_LOAD); d; d = dlls.inext ())
	{
	  debug_printf ("copying data/bss for a loaded dll");
	  if (!child_copy (hchild, true, !*with_forkables,
			   "loaded dll data", d->p.data_start, d->p.data_end,
			   "loaded dll bss", d->p.bss_start, d->p.bss_end,
			   NULL))
	    {
	      this_errno = get_errno ();
#ifdef DEBUGGING
	      error ("copying data/bss for a loaded dll");
#endif
	      goto cleanup;
	    }
	}
    }

  /* Do not attach to the child before it has successfully initialized.
     Otherwise we may wait forever, or deliver an orphan SIGCHILD. */
  if (!child.reattach ())
    {
      this_errno = EAGAIN;
#ifdef DEBUGGING0
      error ("child reattach failed");
#endif
      goto cleanup;
    }

  /* Finally start the child up. */
  resume_child (forker_finished);

  ForceCloseHandle (forker_finished);
  forker_finished = NULL;

  return child_pid;

/* Common cleanup code for failure cases */
cleanup:
  /* release procinfo before hProcess in destructor */
  child.allow_remove ();

  if (fix_impersonation)
    cygheap->user.reimpersonate ();
  if (locked)
    __malloc_unlock ();

  /* Remember to de-allocate the fd table. */
  if (hchild)
    {
      TerminateProcess (hchild, 1);
      if (!child.hProcess) /* no child.procinfo */
	ForceCloseHandle1 (hchild, childhProc);
    }
  if (forker_finished)
    ForceCloseHandle (forker_finished);
  debug_printf ("returning -1");
  return -1;
}

extern "C" int
fork ()
{
  bool with_forkables = false; /* do not force hardlinks on first try */
  int res = dofork (&with_forkables);
  if (res >= 0)
    return res;
  if (with_forkables)
    return res; /* no need for second try when already enabled */
  with_forkables = true; /* enable hardlinks for second try */
  return dofork (&with_forkables);
}

static int
dofork (bool *with_forkables)
{
  frok grouped (with_forkables);

  debug_printf ("entering");
  grouped.load_dlls = 0;

  int res;
  bool ischild = false;

  myself->set_has_pgid_children ();

  if (grouped.ch.parent == NULL)
    return -1;
  if (grouped.ch.subproc_ready == NULL)
    {
      system_printf ("unable to allocate subproc_ready event, %E");
      return -1;
    }

  {
    hold_everything held_everything (ischild);
    /* This tmp_pathbuf constructor is required here because the below setjmp
       magic will otherwise not restore the original buffer count values in
       the thread-local storage.  A process forking too deeply will run into
       the problem to be out of temporary TLS path buffers. */
    tmp_pathbuf tp;

    if (!held_everything)
      {
	if (exit_state)
	  Sleep (INFINITE);
	set_errno (EAGAIN);
	return -1;
      }

    /* Put the dll list in topological dependency ordering, in
       hopes that the child will have a better shot at loading dlls
       properly if it only has to deal with one at a time.  */
    dlls.topsort ();

    ischild = !!setjmp (grouped.ch.jmp);

    volatile char * volatile stackp;
#ifdef __x86_64__
    __asm__ volatile ("movq %%rsp,%0": "=r" (stackp));
#else
    __asm__ volatile ("movl %%esp,%0": "=r" (stackp));
#endif

    if (!ischild)
      res = grouped.parent (stackp);
    else
      {
	res = grouped.child (stackp);
	in_forkee = false;
	ischild = true;	/* might have been reset by fork mem copy */
      }
  }

  if (ischild)
    {
      myself->process_state |= PID_ACTIVE;
      myself->process_state &= ~(PID_INITIALIZING | PID_EXITED | PID_REAPED);
    }
  else if (res < 0)
    {
      if (!grouped.errmsg)
	syscall_printf ("fork failed - child pid %d, errno %d", grouped.child_pid, grouped.this_errno);
      else if (grouped.ch.silentfail ())
	debug_printf ("child %d - %s, errno %d", grouped.child_pid,
		       grouped.errmsg, grouped.this_errno);
      else
	system_printf ("child %d - %s, errno %d", grouped.child_pid,
		       grouped.errmsg, grouped.this_errno);

      set_errno (grouped.this_errno);
    }
  syscall_printf ("%R = fork()", res);
  return res;
}
#ifdef DEBUGGING
void
fork_init ()
{
}
#endif /*DEBUGGING*/


extern "C" int
vfork ()
{
  debug_printf ("stub called");
  return fork ();
}

/* Copy memory from one process to another. */

bool
child_copy (HANDLE hp, bool write, bool silentfail, ...)
{
  va_list args;
  va_start (args, silentfail);
  static const char *huh[] = {"read", "write"};

  char *what;
  while ((what = va_arg (args, char *)))
    {
      char *low = va_arg (args, char *);
      char *high = va_arg (args, char *);
      SIZE_T todo = high - low;
      char *here;

      for (here = low; here < high; here += todo)
	{
	  SIZE_T done = 0;
	  if (here + todo > high)
	    todo = high - here;
	  BOOL res;
	  if (write)
	    res = WriteProcessMemory (hp, here, here, todo, &done);
	  else
	    res = ReadProcessMemory (hp, here, here, todo, &done);
	  debug_printf ("%s - hp %p low %p, high %p, res %d", what, hp, low, high, res);
	  if (!res || todo != done)
	    {
	      if (!res)
		__seterrno ();
	      if (silentfail)
		debug_printf ("%s %s copy failed, %p..%p, done %lu, windows pid %u, %E",
			     what, huh[write], low, high, done, myself->dwProcessId);
	      else
		/* If this happens then there is a bug in our fork
		   implementation somewhere. */
		system_printf ("%s %s copy failed, %p..%p, done %lu, windows pid %u, %E",
			      what, huh[write], low, high, done, myself->dwProcessId);
	      goto err;
	    }
	}
    }

  va_end (args);
  debug_printf ("done");
  return true;

 err:
  va_end (args);
  TerminateProcess (hp, 1);
  set_errno (EAGAIN);
  return false;
}
