/* fork.cc

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include "winsup.h"
#include "dll_init.h"

DWORD NO_COPY chunksize = 0;
/* Timeout to wait for child to start, parent to init child, etc.  */
/* FIXME: Once things stabilize, bump up to a few minutes.  */
#define FORK_WAIT_TIMEOUT (300 * 1000)     /* 300 seconds */

#define dll_data_start &_data_start__
#define dll_data_end &_data_end__
#define dll_bss_start &_bss_start__
#define dll_bss_end &_bss_end__

void
per_thread::set (void *s)
  {
    if (s == PER_THREAD_FORK_CLEAR)
      {
        tls = TlsAlloc ();
	s = NULL;
      }
    TlsSetValue (get_tls (), s);
  }

static void
stack_base (child_info_fork &ch)
{
  MEMORY_BASIC_INFORMATION m;
  memset (&m, 0, sizeof m);
  if (!VirtualQuery ((LPCVOID) &m, &m, sizeof m))
    system_printf ("couldn't get memory info, %E");

  ch.stacktop = m.AllocationBase;
  ch.stackbottom = (LPBYTE) m.BaseAddress + m.RegionSize;
  ch.stacksize = (DWORD) ch.stackbottom - (DWORD) &m;
  debug_printf ("bottom %p, top %p, stack %p, size %d, reserve %d",
		ch.stackbottom, ch.stacktop, &m, ch.stacksize,
		(DWORD) ch.stackbottom - (DWORD) ch.stacktop);
}

/* Copy memory from parent to child.
   The result is a boolean indicating success.  */

static int
fork_copy (PROCESS_INFORMATION &pi, const char *what, ...)
{
  va_list args;
  char *low;
  int pass = 0;

  va_start (args, what);

  while ((low = va_arg (args, char *)))
    {
      char *high = va_arg (args, char *);
      DWORD todo = chunksize ?: high - low;
      char *here;

      for (here = low; here < high; here += todo)
	{
	  DWORD done = 0;
	  if (here + todo > high)
	    todo = high - here;
	  int res = WriteProcessMemory (pi.hProcess, here, here, todo, &done);
	  debug_printf ("child handle %p, low %p, high %p, res %d", pi.hProcess,
			low, high, res);
	  if (!res || todo != done)
	    {
	      if (!res)
		__seterrno ();
	      /* If this happens then there is a bug in our fork
		 implementation somewhere. */
	      system_printf ("%s pass %d failed, %p..%p, done %d, %E",
			    what, pass, low, high, done);
	      goto err;
	    }
	}

      pass++;
    }

  debug_printf ("done");
  return 1;

err:
  TerminateProcess (pi.hProcess, 1);
  set_errno (EAGAIN);
  return 0;
}

/* Wait for child to finish what it's doing and signal us.
   We don't want to wait forever here.If there's a problem somewhere
   it'll hang the entire system (since all forks are mutex'd). If we
   time out, set errno = EAGAIN and hope the app tries again.  */
static int
sync_with_child (PROCESS_INFORMATION &pi, HANDLE subproc_ready,
		 BOOL hang_child, const char *s)
{
  /* We also add the child process handle to the wait. If the child fails
     to initialize (eg. because of a missing dll). Then this
     handle will become signalled. This stops a *looong* timeout wait.
  */
  HANDLE w4[2];

  debug_printf ("waiting for child.  reason: %s", s);
  w4[1] = pi.hProcess;
  w4[0] = subproc_ready;
  DWORD rc = WaitForMultipleObjects (2, w4, FALSE, FORK_WAIT_TIMEOUT);

  if (rc == WAIT_OBJECT_0 ||
      WaitForSingleObject (subproc_ready, 0) == WAIT_OBJECT_0)
    /* That's ok */;
  else if (rc == WAIT_FAILED || rc == WAIT_TIMEOUT)
    {
      if (rc != WAIT_FAILED)
	system_printf ("WaitForMultipleObjects timed out");
      else
	system_printf ("WaitForMultipleObjects failed, %E");
      set_errno (EAGAIN);
      syscall_printf ("-1 = fork(), WaitForMultipleObjects failed");
      TerminateProcess (pi.hProcess, 1);
      return 0;
    }
  else
    {
      /* Child died. Clean up and exit. */
      DWORD errcode;
      GetExitCodeProcess (pi.hProcess, &errcode);
      /* Fix me.  This is not enough.  The fork should not be considered
       * to have failed if the process was essentially killed by a signal.
       */
      if (errcode != STATUS_CONTROL_C_EXIT)
	{
	    system_printf ("child %d(%p) died before initialization with status code %p",
			  pi.dwProcessId, pi.hProcess, errcode);
	    system_printf ("*** child state %s", s);
#ifdef DEBUGGING
	    abort ();
#endif
	}
      set_errno (EAGAIN);
      syscall_printf ("Child died before subproc_ready signalled");
      return 0;
    }

  debug_printf ("child signalled me");
  if (hang_child)
    {
      int n = SuspendThread (pi.hThread);
      debug_printf ("suspend count %d", n); \
    }
  return 1;
}

static int
resume_child (PROCESS_INFORMATION &pi, HANDLE forker_finished)
{
  int rc;

  debug_printf ("here");
  SetEvent (forker_finished);

  rc = ResumeThread (pi.hThread);

  debug_printf ("rc %d", rc);
  if (rc == 1)
    return 1;		// Successful resumption

  /* Can't resume the thread.  Not sure why this would happen unless
     there's a bug in the system.  Things seem to be working OK now
     though, so flag this with EAGAIN, but print a message on the
     console.  */
  small_printf ("fork: ResumeThread failed, rc = %d, %E\n", rc);
  set_errno (EAGAIN);
  syscall_printf ("-1 = fork(), ResumeThread failed");
  TerminateProcess (pi.hProcess, 1);
  return 0;
}

/* Notify parent that it is time for the next step.
   Note that this has to be a macro since the parent may be messing with
   our stack. */
#define sync_with_parent(s, hang_self) \
((void) ({ \
  debug_printf ("signalling parent: %s", s); \
  /* Tell our parent we're waiting. */ \
  if (!SetEvent (child_proc_info->subproc_ready)) \
    api_fatal ("fork child - SetEvent failed, %E"); \
  if (hang_self) \
    { \
      /* Wait for the parent to fill in our stack and heap. \
	 Don't wait forever here.  If our parent dies we don't want to clog \
	 the system.  If the wait fails, we really can't continue so exit.  */ \
      DWORD psync_rc = WaitForSingleObject (child_proc_info->forker_finished, FORK_WAIT_TIMEOUT); \
      switch (psync_rc) \
	{ \
	case WAIT_TIMEOUT: \
	  api_fatal ("sync_with_parent - WFSO timed out"); \
	  break; \
	case WAIT_FAILED: \
	  if (GetLastError () == ERROR_INVALID_HANDLE && \
	      WaitForSingleObject (child_proc_info->forker_finished, 1) != WAIT_FAILED) \
	    break; \
	  api_fatal ("sync_with_parent - WFSO failed, fork_finished %p, %E", child_proc_info->forker_finished); \
	  break; \
	default: \
	  break; \
	} \
      debug_printf ("awake"); \
    } \
  0; \
}))

static volatile void grow_stack_slack();

static void *
stack_dummy (int here)
{
  return &here;
}

extern "C" int
fork ()
{
  int res;
  DWORD rc;
  HANDLE hParent;
  pinfo *child;
  HANDLE subproc_ready, forker_finished;
  void *stack_here;
  int x;
  PROCESS_INFORMATION pi = {0, NULL, 0, 0};

  MALLOC_CHECK;

  /* FIXME: something is broken when copying the stack from the parent
     to the child; we try various tricks here to make sure that the
     stack is good enough to prevent page faults, but the true cause
     is still unknown.  DJ */
  volatile char dummy[4096];
  dummy[0] = dummy[4095] = 0;	// Just to leave some slack in the stack

  grow_stack_slack ();

  debug_printf ("entering");
  /* Calculate how much of stack to copy to child */
  stack_here = stack_dummy (0);

  if (ISSTATE(myself, PID_SPLIT_HEAP))
    {
      system_printf ("The heap has been split, CYGWIN can't fork this process.");
      system_printf ("Increase the heap_chunk_size in the registry and try again.");
      set_errno (ENOMEM);
      syscall_printf ("-1 = fork (), split heap");
      return -1;
    }

  /* Don't start the fork until we have the lock.  */
  child = cygwin_shared->p.allocate_pid ();
  if (!child)
    {
      set_errno (EAGAIN);
      syscall_printf ("-1 = fork (), process table full");
      return -1;
    }

  static child_info_fork ch;
  x = setjmp (ch.jmp);

  if (x == 0)
    {

      /* This will help some of the confusion.  */
      fflush (stdout);

      debug_printf ("parent pid %d, child pid %d", myself->pid, child->pid);

      subproc_ready = CreateEvent (&sec_all, FALSE, FALSE, NULL);
      forker_finished = CreateEvent (&sec_all, FALSE, FALSE, NULL);
      ProtectHandle (subproc_ready);
      ProtectHandle (forker_finished);

      /* If we didn't obtain all the resources we need to fork, allow the program
	 to continue, but record the fact that fork won't work.  */
      if (forker_finished == NULL || subproc_ready == NULL)
	{
	  system_printf ("unable to allocate fork() resources.");
	  system_printf ("fork() disabled.");
	  return -1;
	}

      subproc_init ();

      debug_printf ("about to call setjmp");
      /* Parent.  */
#ifdef DEBUGGING
      /* The ProtectHandle call allocates memory so we need to make sure
	 that enough is set aside here so that the sbrk pointer does not
	 move when ProtectHandle is called after the child is started.
	 Otherwise the sbrk pointers in the parent will not agree with
	 the child and when user_data is (regrettably) copied over,
	 the user_data->ptr field will not be accurate. */
      free (malloc (4096));
#endif

      init_child_info (PROC_FORK1, &ch, child->pid, subproc_ready);

      ch.forker_finished = forker_finished;
      ch.heaptop = user_data->heaptop;
      ch.heapbase = user_data->heapbase;
      ch.heapptr = user_data->heapptr;

      stack_base (ch);

      /* Initialize things that are done later in dll_crt0_1 that aren't done
	 for the forkee.  */
      strcpy(child->progname, myself->progname);

      STARTUPINFO si = {0, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL};

      si.cb = sizeof (STARTUPINFO);
      si.lpReserved2 = (LPBYTE)&ch;
      si.cbReserved2 = sizeof(ch);

      int c_flags = GetPriorityClass (hMainProc) /*|
		    CREATE_NEW_PROCESS_GROUP*/;

      /* If we don't have a console, then don't create a console for the
	 child either.  */
      HANDLE console_handle = CreateFileA ("CONOUT$", GENERIC_WRITE,
					   FILE_SHARE_WRITE, &sec_none_nih,
					   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
					   NULL);

      syscall_printf ("CreateProcessA (%s, %s,0,0,1,%x, 0,0,%p,%p)",
		      myself->progname, myself->progname, c_flags, &si, &pi);
      if (console_handle != INVALID_HANDLE_VALUE && console_handle != 0)
	CloseHandle (console_handle);
      else
	c_flags |= DETACHED_PROCESS;

      hParent = NULL;
      if (!DuplicateHandle (hMainProc, hMainProc, hMainProc, &hParent, 0, 1,
			   DUPLICATE_SAME_ACCESS))
	{
	  system_printf ("couldn't create handle to myself for child, %E");
	  goto cleanup;
	}

      rc = CreateProcessA (myself->progname, /* image to run */
			   myself->progname, /* what we send in arg0 */
			   &sec_none_nih,  /* process security attrs */
			   &sec_none_nih,  /* thread security attrs */
			   TRUE,	  /* inherit handles from parent */
			   c_flags,
			   NULL,	  /* environment filled in later */
			   0,		  /* use current drive/directory */
			   &si,
			   &pi);

      CloseHandle (hParent);

      if (!rc)
	{
	  __seterrno ();
	  syscall_printf ("-1 = fork(), CreateProcessA failed");
	  child->process_state = PID_NOT_IN_USE;
	  ForceCloseHandle(subproc_ready);
	  ForceCloseHandle(forker_finished);
	  subproc_ready = forker_finished = NULL;
	  return -1;
	}

      ProtectHandle (pi.hThread);
      /* Protect the handle but name it similarly to the way it will
	 be called in subproc handling. */
      ProtectHandle1 (pi.hProcess, childhProc);

      /* Fill in fields in the child's process table entry.  */
      child->ppid = myself->pid;
      child->hProcess = pi.hProcess;
      child->dwProcessId = pi.dwProcessId;
      child->uid = myself->uid;
      child->gid = myself->gid;
      child->pgid = myself->pgid;
      child->sid = myself->sid;
      child->ctty = myself->ctty;
      child->umask = myself->umask;
      child->copysigs(myself);
      child->process_state |= PID_INITIALIZING |
			      (myself->process_state & PID_USETTY);
      memcpy (child->username, myself->username, MAX_USER_NAME);
      child->psid = myself->psid;
      memcpy (child->sidbuf, myself->sidbuf, 40);
      memcpy (child->logsrv, myself->logsrv, 256);
      memcpy (child->domain, myself->domain, MAX_COMPUTERNAME_LENGTH+1);
      set_child_mmap_ptr (child);

      /* Wait for subproc to initialize itself. */
      if (!sync_with_child(pi, subproc_ready, TRUE, "waiting for longjmp"))
	goto cleanup;

      /* CHILD IS STOPPED */
      debug_printf ("child is alive (but stopped)");

      /* Initialize, in order: data, bss, heap, stack, dll data, dll bss
	 Note: variables marked as NO_COPY will not be copied
	 since they are placed in a protected segment. */


      MALLOC_CHECK;
      rc = fork_copy (pi, "user/cygwin data",
		      user_data->data_start, user_data->data_end,
		      user_data->bss_start, user_data->bss_end,
		      ch.heapbase, ch.heapptr,
		      stack_here, ch.stackbottom,
		      dll_data_start, dll_data_end,
		      dll_bss_start, dll_bss_end, NULL);

      MALLOC_CHECK;
      if (!rc)
	goto cleanup;

      /* Now fill data/bss of linked dll */
      DO_LINKED_DLL (p)
      {
	debug_printf ("copying data/bss of a linked dll");
	if (!fork_copy (pi, "linked dll data/bss", p->data_start, p->data_end,
						   p->bss_start, p->bss_end,
						   NULL))
	  goto cleanup;
      }
      DLL_DONE;

      proc_register (child);
      int load_dll = DllList::the().forkeeMustReloadDlls() &&
		     DllList::the().numberOfOpenedDlls();

      /* Start thread, and wait for it to reload dlls.  */
      if (!resume_child (pi, forker_finished) ||
	  !sync_with_child (pi, subproc_ready, load_dll, "child loading dlls"))
	goto cleanup;

      /* child reload dlls & then write their data and bss */
      if (load_dll)
      {
	/* CHILD IS STOPPED */
	/* write memory of reloaded dlls */
	DO_LOADED_DLL (p)
	{
	  debug_printf ("copying data/bss for a loaded dll");
	  if (!fork_copy (pi, "loaded dll data/bss", p->data_start, p->data_end,
						     p->bss_start, p->bss_end,
						     NULL))
	    goto cleanup;
	}
	DLL_DONE;
	/* Start the child up again. */
	(void) resume_child (pi, forker_finished);
      }

      ForceCloseHandle (subproc_ready);
      ForceCloseHandle (pi.hThread);
      ForceCloseHandle (forker_finished);
      forker_finished = NULL;
      pi.hThread = NULL;

      res = child->pid;
    }
  else
    {
      /**** Child *****/

      /* We arrive here via a longjmp from "crt0".  */
      (void) stack_dummy (0);		// Just to make sure
      debug_printf ("child is running %d", x);

      debug_printf ("self %p, pid %d, ppid %d",
		    myself, x, myself ? myself->ppid : -1);

      sync_with_parent ("after longjmp.", TRUE);
      ProtectHandle (hParent);

#ifdef DEBUGGING
      char c;
      if (GetEnvironmentVariable ("FORKDEBUG", &c, 1))
	try_to_debug ();
#endif

      /* If we've played with the stack, stacksize != 0.  That means that
	 fork() was invoked from other than the main thread.  Make sure that
	 when the "main" thread exits it calls do_exit, like a normal process.
	 Exit with a status code of 0. */
      if (child_proc_info->stacksize)
	{
	  ((DWORD *)child_proc_info->stackbottom)[-17] = (DWORD)do_exit;
	  ((DWORD *)child_proc_info->stackbottom)[-15] = (DWORD)0;
	}

      MALLOC_CHECK;

      dtable.fixup_after_fork (hParent);
      signal_fixup_after_fork ();
      ForceCloseHandle (hParent);

      MALLOC_CHECK;

      /* reload dlls if necessary */
      if (!DllList::the().forkeeMustReloadDlls() ||
	  !DllList::the().numberOfOpenedDlls())
	sync_with_parent ("performed fork fixup.", FALSE);
      else
	{
	  DllList::the().forkeeLoadDlls();
	  sync_with_parent ("loaded dlls", TRUE);
	}

      (void) ForceCloseHandle (child_proc_info->subproc_ready);
      (void) ForceCloseHandle (child_proc_info->forker_finished);

      if (recreate_mmaps_after_fork (myself->mmap_ptr))
	api_fatal ("recreate_mmaps_after_fork_failed");

      res = 0;
      /* Set thread local stuff to zero.  Under Windows 95/98 this is sometimes
	 non-zero, for some reason.
	 FIXME:  There is a memory leak here after a fork. */
      for (per_thread **t = threadstuff; *t; t++)
	if ((*t)->clear_on_fork ())
	  (*t)->set ();

      /* Initialize signal/process handling */
      sigproc_init ();
    }


  MALLOC_CHECK;
  syscall_printf ("%d = fork()", res);
  return res;

/* Common cleanup code for failure cases */
cleanup:
  /* Remember to de-allocate the fd table. */
  child->process_state = PID_NOT_IN_USE;
  if (pi.hProcess)
    ForceCloseHandle1 (pi.hProcess, childhProc);
  if (pi.hThread)
    ForceCloseHandle (pi.hThread);
  if (subproc_ready)
    ForceCloseHandle (subproc_ready);
  if (forker_finished)
    ForceCloseHandle (forker_finished);
  forker_finished = subproc_ready = child->hProcess = NULL;
  return -1;
}

static volatile void
grow_stack_slack ()
{
  volatile char dummy[16384];
  dummy[0] = dummy[16383] = 0;	// Just to make some slack in the stack
}

#ifdef NEWVFORK
/* Dummy function to force second assignment below to actually be
   carried out */
static vfork_save *
get_vfork_val ()
{
  return vfork_storage.val ();
}
#endif

extern "C"
int
vfork ()
{
#ifndef NEWVFORK
  return fork ();
#else
  vfork_save *vf = get_vfork_val ();

  if (vf == NULL)
    vf = vfork_storage.create ();

  if (!setjmp (vf->j))
    {
      vf->pid = -1;
      __asm__ volatile ("movl %%ebp,%0": "=r" (vf->vfork_ebp):);
      __asm__ volatile ("movl (%%ebp),%0": "=r" (vf->caller_ebp):);
      __asm__ volatile ("movl 4(%%ebp),%0": "=r" (vf->retaddr):);
      return dtable.vfork_child_dup () ? 0 : -1;
    }

  dtable.vfork_parent_restore ();

  vf = get_vfork_val ();
  if (vf->pid < 0)
    {
      int exitval = -vf->pid;
      if ((vf->pid = fork ()) == 0)
	exit (exitval);
    }

  vf->vfork_ebp[0] = vf->caller_ebp;
  vf->vfork_ebp[1] = vf->retaddr;
  return vf->pid;
#endif
}
