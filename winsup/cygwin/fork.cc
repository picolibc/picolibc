/* fork.cc

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include "fhandler.h"
#include "dtable.h"
#include "cygerrno.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "cygheap.h"
#include "child_info.h"
#define NEED_VFORK
#include "perthread.h"
#include "perprocess.h"
#include "dll_init.h"
#include "security.h"

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

  debug_printf ("waiting for child.  reason: %s, hang_child %d", s,
		hang_child);
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
  return 1;
}

static int
resume_child (PROCESS_INFORMATION &pi, HANDLE forker_finished)
{
  SetEvent (forker_finished);
  debug_printf ("signalled child");
  return 1;
}

/* Notify parent that it is time for the next step.
   Note that this has to be a macro since the parent may be messing with
   our stack. */
static void __stdcall
sync_with_parent(const char *s, bool hang_self)
{
  debug_printf ("signalling parent: %s", s);
  /* Tell our parent we're waiting. */
  if (!SetEvent (child_proc_info->subproc_ready))
    api_fatal ("fork child - SetEvent failed, %E");
  if (hang_self)
    {
      HANDLE h = child_proc_info->forker_finished;
      /* Wait for the parent to fill in our stack and heap.
	 Don't wait forever here.  If our parent dies we don't want to clog
	 the system.  If the wait fails, we really can't continue so exit.  */
      DWORD psync_rc = WaitForSingleObject (h, FORK_WAIT_TIMEOUT);
      debug_printf ("awake");
      switch (psync_rc)
	{
	case WAIT_TIMEOUT:
	  api_fatal ("WFSO timed out");
	  break;
	case WAIT_FAILED:
	  if (GetLastError () == ERROR_INVALID_HANDLE &&
	      WaitForSingleObject (child_proc_info->forker_finished, 1) != WAIT_FAILED)
	    break;
	  api_fatal ("WFSO failed, fork_finished %p, %E", child_proc_info->forker_finished);
	  break;
	default:
	  debug_printf ("no problems");
	  break;
	}
    }
}

static int __stdcall
fork_child (HANDLE& hParent, dll *&first_dll, bool& load_dlls)
{
  debug_printf ("child is running.  pid %d, ppid %d, stack here %p",
		myself->pid, myself->ppid, __builtin_frame_address (0));

  /* Restore the inheritance state as in parent
     Don't call setuid here! The flags are already set. */
  if (myself->impersonated)
    {
      debug_printf ("Impersonation of child, token: %d", myself->token);
      if (myself->token == INVALID_HANDLE_VALUE)
	RevertToSelf (); // probably not needed
      else if (!ImpersonateLoggedOnUser (myself->token))
	system_printf ("Impersonate for forked child failed: %E");
    }

  sync_with_parent ("after longjmp.", TRUE);
  ProtectHandle (hParent);
  sigproc_printf ("hParent %p, child 1 first_dll %p, load_dlls %d\n", hParent, first_dll, load_dlls);

#ifdef DEBUGGING
  char c;
  if (GetEnvironmentVariable ("FORKDEBUG", &c, 1))
    try_to_debug ();
  char buf[80];
  /* This is useful for debugging fork problems.  Use gdb to attach to
     the pid reported here. */
  if (GetEnvironmentVariable ("CYGWIN_FORK_SLEEP", buf, sizeof (buf)))
    {
      small_printf ("Sleeping %d after fork, pid %u\n", atoi (buf), GetCurrentProcessId ());
      Sleep (atoi(buf));
    }
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

  pinfo_fixup_after_fork ();
  fdtab.fixup_after_fork (hParent);
  signal_fixup_after_fork ();

  MALLOC_CHECK;

  /* If we haven't dynamically loaded any dlls, just signal
     the parent.  Otherwise, load all the dlls, tell the parent
      that we're done, and wait for the parent to fill in the.
      loaded dlls' data/bss. */
  if (!load_dlls)
    sync_with_parent ("performed fork fixup.", FALSE);
  else
    {
      dlls.load_after_fork (hParent, first_dll);
      sync_with_parent ("loaded dlls", TRUE);
    }

  ForceCloseHandle (hParent);
  (void) ForceCloseHandle (child_proc_info->subproc_ready);
  (void) ForceCloseHandle (child_proc_info->forker_finished);

  if (recreate_mmaps_after_fork (myself->mmap_ptr))
    api_fatal ("recreate_mmaps_after_fork_failed");

  /* Set thread local stuff to zero.  Under Windows 95/98 this is sometimes
     non-zero, for some reason.
     FIXME:  There is a memory leak here after a fork. */
  for (per_thread **t = threadstuff; *t; t++)
    if ((*t)->clear_on_fork ())
      (*t)->set ();

  /* Initialize signal/process handling */
  sigproc_init ();
  cygbench ("fork-child");
  return 0;
}

static int __stdcall
fork_parent (void *stack_here, HANDLE& hParent, dll *&first_dll, bool& load_dlls, child_info_fork &ch)
{
  HANDLE subproc_ready, forker_finished;
  DWORD rc;
  PROCESS_INFORMATION pi = {0, NULL, 0, 0};
  static NO_COPY HANDLE last_fork_proc = NULL;

  subproc_init ();

#ifdef DEBUGGING
  /* The ProtectHandle call allocates memory so we need to make sure
     that enough is set aside here so that the sbrk pointer does not
     move when ProtectHandle is called after the child is started.
     Otherwise the sbrk pointers in the parent will not agree with
     the child and when user_data is (regrettably) copied over,
     the user_data->ptr field will not be accurate. */
  free (malloc (4096));
#endif

  int c_flags = GetPriorityClass (hMainProc) /*|
		CREATE_NEW_PROCESS_GROUP*/;
  STARTUPINFO si = {0, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL};

  /* If we don't have a console, then don't create a console for the
     child either.  */
  HANDLE console_handle = CreateFileA ("CONOUT$", GENERIC_WRITE,
				       FILE_SHARE_WRITE, &sec_none_nih,
				       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
				       NULL);

  if (console_handle != INVALID_HANDLE_VALUE && console_handle != 0)
    CloseHandle (console_handle);
  else
    c_flags |= DETACHED_PROCESS;

  hParent = NULL;
  if (!DuplicateHandle (hMainProc, hMainProc, hMainProc, &hParent, 0, 1,
			DUPLICATE_SAME_ACCESS))
    {
      system_printf ("couldn't create handle to myself for child, %E");
      return -1;
    }

  /* Remember the address of the first loaded dll and decide
     if we need to load dlls.  We do this here so that this
     information will be available in the parent and, when
     the stack is copied, in the child. */
  first_dll = dlls.start.next;
  load_dlls = dlls.reload_on_fork && dlls.loaded_dlls;

  /* This will help some of the confusion.  */
  fflush (stdout);

  subproc_ready = CreateEvent (&sec_all, FALSE, FALSE, NULL);
  if (subproc_ready == NULL)
    {
      CloseHandle (hParent);
      system_printf ("unable to allocate subproc_ready event, %E");
      return -1;
    }
  forker_finished = CreateEvent (&sec_all, FALSE, FALSE, NULL);
  if (forker_finished == NULL)
    {
      CloseHandle (hParent);
      CloseHandle (subproc_ready);
      system_printf ("unable to allocate subproc_ready event, %E");
      return -1;
    }

  ProtectHandle (subproc_ready);
  ProtectHandle (forker_finished);

  init_child_info (PROC_FORK1, &ch, 1, subproc_ready);

  ch.forker_finished = forker_finished;
  ch.heaptop = user_data->heaptop;
  ch.heapbase = user_data->heapbase;
  ch.heapptr = user_data->heapptr;

  stack_base (ch);

  si.cb = sizeof (STARTUPINFO);
  si.lpReserved2 = (LPBYTE)&ch;
  si.cbReserved2 = sizeof(ch);

  /* Remove impersonation */
  uid_t uid;
  uid = geteuid();
  if (myself->impersonated && myself->token != INVALID_HANDLE_VALUE)
    seteuid (myself->orig_uid);

  ch.parent = hParent;
  ch.cygheap = cygheap;
  ch.cygheap_max = cygheap_max;

  char sa_buf[1024];
  syscall_printf ("CreateProcess (%s, %s, 0, 0, 1, %x, 0, 0, %p, %p)",
		  myself->progname, myself->progname, c_flags, &si, &pi);
  rc = CreateProcess (myself->progname, /* image to run */
		      myself->progname, /* what we send in arg0 */
		      allow_ntsec ? sec_user (sa_buf) : &sec_none_nih,
		      allow_ntsec ? sec_user (sa_buf) : &sec_none_nih,
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
      syscall_printf ("CreateProcessA failed, %E");
      ForceCloseHandle(subproc_ready);
      ForceCloseHandle(forker_finished);
      /* Restore impersonation */
      if (myself->impersonated && myself->token != INVALID_HANDLE_VALUE)
	seteuid (uid);
      return -1;
    }

  pinfo forked (cygwin_pid (pi.dwProcessId), 1);

  /* Initialize things that are done later in dll_crt0_1 that aren't done
     for the forkee.  */
  strcpy(forked->progname, myself->progname);

  /* Restore impersonation */
  if (myself->impersonated && myself->token != INVALID_HANDLE_VALUE)
    seteuid (uid);

  ProtectHandle (pi.hThread);
  /* Protect the handle but name it similarly to the way it will
     be called in subproc handling. */
  ProtectHandle1 (pi.hProcess, childhProc);

  /* Keep a handle to the current forked process sitting around to prevent
     Windows from reusing the same pid twice in a row.  Having the same pid
     twice in a row confuses bash.  So, after every CreateProcess, we can safely
     remove the old pid and save a handle to the newly created process.  Keeping
     a handle open will stop windows from reusing the same pid.  */
  if (last_fork_proc)
    CloseHandle (last_fork_proc);
  if (!DuplicateHandle (hMainProc, pi.hProcess, hMainProc, &last_fork_proc,
			0, FALSE, DUPLICATE_SAME_ACCESS))
    system_printf ("couldn't create last_fork_proc, %E");

  /* Fill in fields in the child's process table entry.  */
  forked->ppid = myself->pid;
  forked->hProcess = pi.hProcess;
  forked->dwProcessId = pi.dwProcessId;
  forked->uid = myself->uid;
  forked->gid = myself->gid;
  forked->pgid = myself->pgid;
  forked->sid = myself->sid;
  forked->ctty = myself->ctty;
  forked->umask = myself->umask;
  forked->copysigs(myself);
  forked->process_state |= PID_INITIALIZING |
			  (myself->process_state & PID_USETTY);
  memcpy (forked->username, myself->username, MAX_USER_NAME);
  if (myself->use_psid)
    {
      memcpy (forked->psid, myself->psid, MAX_SID_LEN);
      forked->use_psid = 1;
    }
  memcpy (forked->logsrv, myself->logsrv, MAX_HOST_NAME);
  memcpy (forked->domain, myself->domain, MAX_COMPUTERNAME_LENGTH+1);
  forked->token = myself->token;
  forked->impersonated = myself->impersonated;
  forked->orig_uid = myself->orig_uid;
  forked->orig_gid = myself->orig_gid;
  forked->real_uid = myself->real_uid;
  forked->real_gid = myself->real_gid;
  strcpy (forked->root, myself->root);
  forked->rootlen = myself->rootlen;
  set_child_mmap_ptr (forked);

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

  /* Now fill data/bss of any DLLs that were linked into the program. */
  for (dll *d = dlls.istart (DLL_LINK); d; d = dlls.inext ())
    {
      debug_printf ("copying data/bss of a linked dll");
      if (!fork_copy (pi, "linked dll data/bss", d->p.data_start, d->p.data_end,
						 d->p.bss_start, d->p.bss_end,
						 NULL))
	goto cleanup;
    }

  forked.remember ();

  /* Start thread, and wait for it to reload dlls.  */
  if (!resume_child (pi, forker_finished) ||
      !sync_with_child (pi, subproc_ready, load_dlls, "child loading dlls"))
    goto cleanup;

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
	  if (!fork_copy (pi, "loaded dll data/bss", d->p.data_start, d->p.data_end,
						     d->p.bss_start, d->p.bss_end,
						     NULL))
	    goto cleanup;
	}
      /* Start the child up again. */
      (void) resume_child (pi, forker_finished);
    }

  ForceCloseHandle (subproc_ready);
  ForceCloseHandle (pi.hThread);
  ForceCloseHandle (forker_finished);
  forker_finished = NULL;
  pi.hThread = NULL;

  return forked->pid;

/* Common cleanup code for failure cases */
cleanup:
  /* Remember to de-allocate the fd table. */
  if (pi.hProcess)
    ForceCloseHandle1 (pi.hProcess, childhProc);
  if (pi.hThread)
    ForceCloseHandle (pi.hThread);
  if (subproc_ready)
    ForceCloseHandle (subproc_ready);
  if (forker_finished)
    ForceCloseHandle (forker_finished);
  return -1;
}

extern "C" int
fork ()
{
  struct
  {
    HANDLE hParent;
    dll *first_dll;
    bool load_dlls;
  } grouped;

  MALLOC_CHECK;

  debug_printf ("entering");
  grouped.hParent = grouped.first_dll = NULL;
  grouped.load_dlls = 0;

  if (ISSTATE(myself, PID_SPLIT_HEAP))
    {
      system_printf ("The heap has been split, CYGWIN can't fork this process.");
      system_printf ("Increase the heap_chunk_size in the registry and try again.");
      set_errno (ENOMEM);
      syscall_printf ("-1 = fork (), split heap");
      return -1;
    }

  void *esp;
  __asm ("movl %%esp,%0": "=r" (esp));

  child_info_fork ch;

  int res = setjmp (ch.jmp);

  if (res)
    res = fork_child (grouped.hParent, grouped.first_dll, grouped.load_dlls);
  else
    res = fork_parent (esp, grouped.hParent, grouped.first_dll, grouped.load_dlls, ch);

  MALLOC_CHECK;
  syscall_printf ("%d = fork()", res);
  return res;
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
  char **esp, **pp;

  if (vf == NULL)
    vf = vfork_storage.create ();

  if (!setjmp (vf->j))
    {
      vf->pid = -1;
      __asm__ volatile ("movl %%esp,%0": "=r" (vf->vfork_esp):);
      __asm__ volatile ("movl %%ebp,%0": "=r" (vf->vfork_ebp):);
      for (pp = (char **)vf->frame, esp = vf->vfork_esp;
	   esp <= vf->vfork_ebp + 1; pp++, esp++)
	*pp = *esp;
      return fdtab.vfork_child_dup () ? 0 : -1;
    }

  fdtab.vfork_parent_restore ();

  vf = get_vfork_val ();
  if (vf->pid < 0)
    {
      int exitval = -vf->pid;
      if ((vf->pid = fork ()) == 0)
	exit (exitval);
    }

  __asm__ volatile ("movl %%esp,%0": "=r" (esp):);
  for (pp = (char **)vf->frame, esp = vf->vfork_esp;
       esp <= vf->vfork_ebp + 1; pp++, esp++)
    *esp = *pp;

  return vf->pid;
#endif
}
