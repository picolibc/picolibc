/* sigproc.cc: inter/intra signal and sub process handler

   Copyright 1997, 1998, 1999, 2000 Cygnus Solutions.

   Written by Christopher Faylor <cgf@cygnus.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include "winsup.h"

extern BOOL allow_ntsec;

/*
 * Convenience defines
 */
#define WSSC		   60000 // Wait for signal completion
#define WPSP		   40000 // Wait for proc_subproc mutex
#define WSPX		   20000 // Wait for wait_sig to terminate
#define WWSP		   20000 // Wait for wait_subproc to terminate

#define WAIT_SIG_PRIORITY		THREAD_PRIORITY_NORMAL

#define TOTSIGS	(NSIG + __SIGOFFSET)

#define sip_printf(fmt, args...) sigproc_printf (fmt , ## args)

#define wake_wait_subproc() SetEvent (events[0])

#define no_signals_available() (!hwait_sig || !sig_loop_wait)

/*
 * Global variables
 */
const char *__sp_fn ;
int __sp_ln;

char NO_COPY myself_nowait_dummy[1] = {'0'};// Flag to sig_send that signal goes to
					//  current process but no wait is required
char NO_COPY myself_nowait_nonmain_dummy[1] = {'1'};// Flag to sig_send that signal goes to
					//  current process but no wait is required
					//  if this is not the main thread.

HANDLE NO_COPY signal_arrived;		// Event signaled when a signal has
					//  resulted in a user-specified
					//  function call
/*
 * Common variables
 */


/* How long to wait for message/signals.  Normally this is infinite.
 * On termination, however, these are set to zero as a flag to exit.
 */

#define Static static NO_COPY

Static DWORD proc_loop_wait = 500;	// Wait for subprocesses to exit
Static DWORD sig_loop_wait = 500;	// Wait for signals to arrive

Static HANDLE sigcatch_nonmain = NULL;	// The semaphore signaled when
					//  signals are available for
					//  processing from non-main thread
Static HANDLE sigcatch_main = NULL;	// Signalled when main thread sends a
					//  signal
Static HANDLE sigcatch_nosync = NULL;	// Signal wait_sig to scan sigtodo
					//  but not to bother with any
					//  synchronization
Static HANDLE sigcomplete_main = NULL;	// Event signaled when a signal has
					//  finished processing for the main
					//  thread
Static HANDLE sigcomplete_nonmain = NULL;// Semaphore raised for non-main
					//  threads when a signal has finished
					//  processing
Static HANDLE hwait_sig = NULL;		// Handle of wait_sig thread
Static HANDLE hwait_subproc = NULL;	// Handle of sig_subproc thread

Static HANDLE wait_sig_inited = NULL;	// Control synchronization of
					//  message queue startup

/* Used by WaitForMultipleObjects.  These are handles to child processes.
 */
Static HANDLE events[PSIZE + 1] = {0};	// All my children's handles++
#define hchildren (events + 1)		// Where the children handles begin
Static pinfo *pchildren[PSIZE] = {NULL};// All my children info
Static pinfo *zombies[PSIZE] = {NULL};	// All my deceased children info
Static int nchildren = 0;		// Number of active children
Static int nzombies = 0;		// Number of deceased children

Static waitq waitq_head = {0, 0, 0, 0, 0, 0, 0};// Start of queue for wait'ing threads
Static waitq waitq_main;		// Storage for main thread

muto NO_COPY *sync_proc_subproc = NULL;	// Control access to subproc stuff

DWORD NO_COPY sigtid = 0;		// ID of the signal thread

int NO_COPY pending_signals = 0;	// TRUE if signals pending

/* Functions
 */
static int __stdcall checkstate (waitq *);
static __inline__ BOOL get_proc_lock (DWORD, DWORD);
static HANDLE __stdcall getsem (pinfo *, const char *, int, int);
static void __stdcall remove_child (int);
static void __stdcall remove_zombie (int);
static DWORD WINAPI wait_sig (VOID *arg);
static int __stdcall stopped_or_terminated (waitq *, pinfo *);
static DWORD WINAPI wait_subproc (VOID *);

/* Determine if the parent process is alive.
 */

BOOL __stdcall
my_parent_is_alive ()
{
  DWORD res;
  if (!parent_alive)
    {
      debug_printf ("No parent_alive mutex");
      res = FALSE;
    }
  else
    for (int i = 0; i < 2; i++)
      switch (res = WaitForSingleObject (parent_alive, 0))
	{
	  case WAIT_OBJECT_0:
	    debug_printf ("parent dead.");
	    res = FALSE;
	    goto out;
	  case WAIT_TIMEOUT:
	    debug_printf ("parent still alive");
	    res = TRUE;
	    goto out;
	  case WAIT_FAILED:
	    DWORD werr = GetLastError ();
	    if (werr == ERROR_INVALID_HANDLE && i == 0)
	      continue;
	    system_printf ("WFSO for parent_alive(%p) failed, error %d",
			   parent_alive, werr);
	    res = FALSE;
	    goto out;
	}
out:
  return res;
}

__inline static void
wait_for_me ()
{
  /* See if this is the first signal call after initialization.
   * If so, wait for notification that all initialization has completed.
   * Then set the handle to NULL to avoid checking this again.
   */
  if (wait_sig_inited)
    {
      (void) WaitForSingleObject (wait_sig_inited, INFINITE);
      (void) ForceCloseHandle (wait_sig_inited);
      wait_sig_inited = NULL;
    }
}

static BOOL __stdcall
proc_can_be_signalled (pinfo *p)
{
  if (p == myself_nowait || p == myself_nowait_nonmain || p == myself)
    {
      wait_for_me ();
      return 1;
    }

  return ISSTATE (p, PID_INITIALIZING) ||
	 (((p)->process_state & (PID_ACTIVE | PID_IN_USE)) ==
	  (PID_ACTIVE | PID_IN_USE));
}

/* Test to determine if a process really exists and is processing
 * signals.
 */
BOOL __stdcall
proc_exists (pinfo *p)
{
  HANDLE h;

  if (p == NULL)
    return FALSE;

  if (p == myself || p == myself_nowait_nonmain || p == myself_nowait)
    return TRUE;

  if (p->process_state == PID_NOT_IN_USE || !p->dwProcessId)
    return FALSE;

  sip_printf ("checking for existence of pid %d, window pid %d", p->pid,
	      p->dwProcessId);
  if (p->ppid == myself->pid && p->hProcess != NULL)
    {
      sip_printf ("it's mine, process_state %x", p->process_state);
      return proc_can_be_signalled (p);
    }

  /* Note: Process is alive if OpenProcess() call fails due to permissions */
  if (((h = OpenProcess (STANDARD_RIGHTS_REQUIRED, FALSE, p->dwProcessId))
      != NULL) || (GetLastError () == ERROR_ACCESS_DENIED))
    {
      sip_printf ("it exists, %p", h);
      if (h)
	{
	  DWORD rc = WaitForSingleObject (h, 0);
	  CloseHandle (h);
	  if (rc == WAIT_OBJECT_0)
	    return 0;
	}
      return proc_can_be_signalled (p);
    }

  sip_printf ("it doesn't exist");
  /* If the parent pid does not exist, clean this process out of the pinfo
   * table.  It must have died abnormally.
   */
  if ((p->pid == p->ppid) || (p->ppid == 1) || !proc_exists (procinfo (p->ppid)))
    {
      p->hProcess = NULL;
      p->process_state = PID_NOT_IN_USE;
    }
  return FALSE;
}

/* Handle all subprocess requests
 */
#define vchild ((pinfo *) val)
int __stdcall
proc_subproc (DWORD what, DWORD val)
{
  int rc = 1;
  int potential_match;
  DWORD exitcode;
  pinfo *child;
  int clearing;
  waitq *w;

#define wval	 ((waitq *) val)

  sip_printf ("args: %x, %d", what, val);

  if (!get_proc_lock (what, val))	// Serialize access to this function
    {
      sip_printf ("I am not ready");
      goto out1;
    }

  switch (what)
    {
    /* Add a new subprocess to the children arrays.
     * (usually called from the main thread)
     */
    case PROC_ADDCHILD:
      if (nchildren >= PSIZE - 1)
	system_printf ("nchildren too large %d", nchildren);
      if (WaitForSingleObject (vchild->hProcess, 0) != WAIT_TIMEOUT)
	{
	  system_printf ("invalid process handle %p.  pid %d, win pid %d",
			vchild->hProcess, vchild->pid, vchild->dwProcessId);
	  rc = 0;
	  break;
	}

      pchildren[nchildren] = vchild;
      hchildren[nchildren] = vchild->hProcess;
      ProtectHandle (vchild->hProcess);
      sip_printf ("added pid %d to wait list, slot %d, winpid %p, handle %p",
		  vchild->pid, nchildren, vchild->dwProcessId,
		  vchild->hProcess);

      nchildren++;
      wake_wait_subproc ();
      break;

    /* A child process had terminated.
     * Possibly this is just due to an exec().  Cygwin implements an exec()
     * as a "handoff" from one windows process to another.  If child->hProcess
     * is different from what is recorded in hchildren, then this is an exec().
     * Otherwise this is a normal child termination event.
     * (called from wait_subproc thread)
     */
    case PROC_CHILDTERMINATED:
      rc = 0;
      child = pchildren[val];
      if (GetExitCodeProcess (hchildren[val], &exitcode) &&
	  hchildren[val] != child->hProcess)
	{
	  sip_printf ("pid %d[%d], reparented old hProcess %p, new %p",
		      child->pid, val, hchildren[val], child->hProcess);
	  ForceCloseHandle1 (hchildren[val], childhProc);
	  hchildren[val] = child->hProcess; /* Filled out by child */
	  ProtectHandle1 (child->hProcess, childhProc);
	  break;			// This was an exec()
	}

      sip_printf ("pid %d[%d] terminated, handle %p, nchildren %d, nzombies %d",
		  child->pid, val, hchildren[val], nchildren, nzombies);
      remove_child (val);		// Remove from children arrays
      zombies[nzombies++] = child;	// Add to zombie array
      child->process_state = PID_ZOMBIE;// Walking dead
      if (!proc_loop_wait)		// Don't bother if wait_subproc is
	break;				//  exiting

      /* Send a SIGCHLD to myself. */
      rc = sig_send (myself_nowait, SIGCHLD);	// Send a SIGCHLD
      break;

    /* A child is in the stopped state.  Scan wait() queue to see if anyone
     * should be notified.  (Called from wait_sig thread)
     */
    case PROC_CHILDSTOPPED:
      child = myself;		// Just to avoid accidental NULL dereference
      sip_printf ("Received stopped notification");
      clearing = 0;
      goto scan_wait;

    /* Clear all waiting threads.  Called from exceptions.cc prior to
     * the main thread's dispatch to a signal handler function.
     * (called from wait_sig thread)
     */
    case PROC_CLEARWAIT:
      /* Clear all "wait"ing threads. */
      if (val)
	sip_printf ("clear waiting threads");
      else
	sip_printf ("looking for processes to reap");
      clearing = val;

    scan_wait:
      /* Scan the linked list of wait()ing threads.  If a wait's parameters
       * match this pid, then activate it.
       */
      for (w = &waitq_head; w->next != NULL; w = w->next)
	{
	  if ((potential_match = checkstate (w)) > 0)
	    sip_printf ("released waiting thread");
	  else if (!clearing && potential_match < 0)
	    sip_printf ("only found non-terminated children");
	  else if (potential_match <= 0)		// nothing matched
	    {
	      sip_printf ("waiting thread found no children");
	      HANDLE oldw = w->next->ev;
	      if (clearing)
		w->next->status = -1;		/* flag that a signal was received */
	      else
		w->next->ev = NULL;
	      if (!SetEvent (oldw))
		system_printf ("couldn't wake up wait event %p, %E", oldw);
	      w->next = w->next->next;
	    }
	  if (w->next == NULL)
	    break;
	}

      if (!clearing)
	sip_printf ("finished processing terminated/stopped child");
      else
	{
	  waitq_head.next = NULL;
	  sip_printf ("finished clearing");
	}
      break;

    /* Handle a wait4() operation.  Allocates an event for the calling
     * thread which is signaled when the appropriate pid exits or stops.
     * (usually called from the main thread)
     */
    case PROC_WAIT:
      wval->ev = NULL;		// Don't know event flag yet

      if (wval->pid <= 0)
	child = NULL;		// Not looking for a specific pid
      else if ((child = procinfo (wval->pid)) == NULL)
	goto out;		// invalid pid.  flag no such child

      wval->status = 0;		// Don't know status yet

      /* Put waitq structure at the end of a linked list. */
      for (w = &waitq_head; w->next != NULL; w = w->next)
	if (w->next == wval && (w->next = w->next->next) == NULL)
	  break;

      wval->next = NULL;	/* This will be last in the list */
      sip_printf ("wval->pid %d, wval->options %d", wval->pid, wval->options);

      /* If the first time for this thread, create a new event, otherwise
       * reset the event.
       */
      if ((wval->ev = wval->thread_ev) == NULL)
	{
	  wval->ev = wval->thread_ev = CreateEvent (&sec_none_nih, TRUE,
						    FALSE, NULL);
	  ProtectHandle (wval->ev);
	}
      ResetEvent (wval->ev);

      /* Scan list of children to see if any have died.
       * If so, the event flag is set so that the wait* ()
       * process will return immediately.
       *
       * If no children were found and the wait option was WNOHANG,
       * then set the pid to 0 and remove the waitq value from
       * consideration.
       */
      w->next = wval;		/* set at end of wait queue */
      if ((potential_match = checkstate (w)) <= 0)
	{
	  if (!potential_match)
	    {
	      w->next = NULL;		// don't want to keep looking
	      wval->ev = NULL;		// flag that there are no children
	      sip_printf ("no appropriate children, %p, %p",
			  wval->thread_ev, wval->ev);
	    }
	  else if (wval->options & WNOHANG)
	    {
	      w->next = NULL;		// don't want to keep looking
	      wval->pid = 0;		// didn't find a pid
	      if (!SetEvent (wval->ev))	// wake up wait4 () immediately
		system_printf ("Couldn't wake up wait event, %E");
	      sip_printf ("WNOHANG and no terminated children, %p, %p",
			  wval->thread_ev, wval->ev);
	    }
	}
      if (w->next != NULL)
	sip_printf ("wait activated %p, %p", wval->thread_ev, wval->ev);
      else if (wval->ev != NULL)
	sip_printf ("wait activated %p.  Reaped zombie.", wval->ev);
      else
	sip_printf ("wait not activated %p, %p", wval->thread_ev, wval->ev);
      break;
  }

out:
  sync_proc_subproc->release ();	// Release the lock
out1:
  sip_printf ("returning %d", rc);
  return rc;
}

/* Terminate the wait_subproc thread.
 * Called on process exit.
 * Also called by spawn_guts to disassociate any subprocesses from this
 * process.  Subprocesses will then know to clean up after themselves and
 * will not become zombies.
 */
void __stdcall
proc_terminate (void)
{
  sip_printf ("nchildren %d, nzombies %d", nchildren, nzombies);
  /* Signal processing is assumed to be blocked in this routine. */
  if (hwait_subproc)
    {
      int rc;
      proc_loop_wait = 0;	// Tell wait_subproc thread to exit
      wake_wait_subproc ();	// Wake wait_subproc loop

      /* Wait for wait_subproc thread to exit (but not *too* long) */
      if ((rc = WaitForSingleObject (hwait_subproc, WWSP)) != WAIT_OBJECT_0)
	if (rc == WAIT_TIMEOUT)
	  system_printf ("WFSO(hwait_subproc) timed out");
	else
	  system_printf ("WFSO(hwait_subproc), rc %d, %E", rc);

      HANDLE h = hwait_subproc;
      hwait_subproc = NULL;
      ForceCloseHandle1 (h, hwait_subproc);

      sync_proc_subproc->acquire(WPSP);
      (void) proc_subproc (PROC_CLEARWAIT, 1);

      lock_pinfo_for_update (INFINITE);
      /* Clean out zombie processes from the pid list. */
      int i;
      for (i = 0; i < nzombies; i++)
	{
	  pinfo *child;
	  if ((child = zombies[i])->hProcess)
	    {
	      ForceCloseHandle1 (child->hProcess, childhProc);
	      child->hProcess = NULL;
	    }
	  child->process_state = PID_NOT_IN_USE;
	}

      /* Disassociate my subprocesses */
      for (i = 0; i < nchildren; i++)
	{
	  pinfo *child;
	  if ((child = pchildren[i])->process_state == PID_NOT_IN_USE)
	    continue;		// Should never happen
	  if (!child->hProcess)
	    sip_printf ("%d(%d) hProcess cleared already?", child->pid,
			child->dwProcessId);
	  else
	    {
	      ForceCloseHandle1 (child->hProcess, childhProc);
	      child->hProcess = NULL;
	      if (!proc_exists (child))
		{
		  sip_printf ("%d(%d) doesn't exist", child->pid,
			      child->dwProcessId);
		  child->process_state = PID_NOT_IN_USE;	/* a reaped child */
		}
	      else
		{
		  sip_printf ("%d(%d) closing active child handle", child->pid,
			      child->dwProcessId);
		  child->ppid = 1;
		  if (child->pgid == myself->pid)
		    child->process_state |= PID_ORPHANED;
		}
	    }
	}
      unlock_pinfo ();
      nchildren = nzombies = 0;

      /* Attempt to close and release sync_proc_subproc in a
       * non-raceable manner.
       */
      muto *m = sync_proc_subproc;
      if (m)
	{
	  sync_proc_subproc = NULL;
	  delete m;
	}
    }
  sip_printf ("leaving");
}

/* Clear pending signal from the sigtodo array
 */
void __stdcall
sig_clear (int sig)
{
  (void) InterlockedExchange (myself->getsigtodo(sig), 0L);
  return;
}

/* Force the wait_sig thread to wake up and scan the sigtodo array.
 */
extern "C" int __stdcall
sig_dispatch_pending (int justwake)
{
  if (!hwait_sig)
    return 0;

  int was_pending = pending_signals;
#ifdef DEBUGGING
  sip_printf ("pending_signals %d", was_pending);
#endif
  if (!was_pending && !justwake)
#ifdef DEBUGGING
    sip_printf ("no need to wake anything up");
#else
    ;
#endif
  else
    {
      wait_for_me ();
      if (!justwake)
	(void) sig_send (myself, __SIGFLUSH);
      else if (ReleaseSemaphore (sigcatch_nosync, 1, NULL))
#ifdef DEBUGGING
	sip_printf ("woke up wait_sig");
#else
	;
#endif
      else if (no_signals_available ())
	/*sip_printf ("I'm going away now")*/;
      else
	system_printf ("%E releasing sigcatch_nosync(%p)", sigcatch_nosync);
 
    }
  return was_pending;
}

/* Message initialization.  Called from dll_crt0_1
 *
 * This routine starts the signal handling thread.  The wait_sig_inited
 * event is used to signal that the thread is ready to handle signals.
 * We don't wait for this during initialization but instead detect it
 * in sig_send to gain a little concurrency.
 */
void __stdcall
sigproc_init ()
{
  wait_sig_inited = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
  ProtectHandle (wait_sig_inited);

  /* local event signaled when main thread has been dispatched
     to a signal handler function. */
  signal_arrived = CreateEvent(&sec_none_nih, TRUE, FALSE, NULL);

  if (!(hwait_sig = makethread (wait_sig, NULL, 0, "sig")))
    {
      system_printf ("cannot create wait_sig thread, %E");
      api_fatal ("terminating");
    }

  ProtectHandle (hwait_sig);

  /* sync_proc_subproc is used by proc_subproc.  It serialises
   * access to the children and zombie arrays.
   */
  sync_proc_subproc = new_muto (FALSE, "sync_proc_subproc");

  /* Initialize waitq structure for main thread.  A waitq structure is
   * allocated for each thread that executes a wait to allow multiple threads
   * to perform waits.  Pre-allocate a waitq structure for the main thread.
   */
  waitq *w;
  if ((w = (waitq *)waitq_storage.get ()) == NULL)
    {
      w = &waitq_main;
      waitq_storage.set (w);
    }
  memset (w, 0, sizeof *w);	// Just to be safe

  sip_printf ("process/signal handling enabled(%x)", myself->process_state);
  return;
}

/* Called on process termination to terminate signal and process threads.
 */
void __stdcall
sigproc_terminate (void)
{
  HANDLE h = hwait_sig;
  hwait_sig = NULL;

  if (GetCurrentThreadId () == sigtid)
    {
      ForceCloseHandle (sigcomplete_main);
      for (int i = 0; i < 20; i++)
	(void) ReleaseSemaphore (sigcomplete_nonmain, 1, NULL);
      ForceCloseHandle (sigcomplete_nonmain);
      ForceCloseHandle (sigcatch_main);
      ForceCloseHandle (sigcatch_nonmain);
      ForceCloseHandle (sigcatch_nosync);
    }
  proc_terminate ();		// Terminate process handling thread

  if (!sig_loop_wait)
    sip_printf ("sigproc_terminate: sigproc handling not active");
  else
    {
      sigproc_printf ("entering");
      sig_loop_wait = 0;	// Tell wait_sig to exit when it is
				//  finished with anything it is doing
      sig_dispatch_pending (TRUE);	// wake up and die

      /* If !hwait_sig, then the process probably hasn't even finished
       * its initialization phase.
       */
      if (hwait_sig)
	{
	  if (GetCurrentThreadId () != sigtid)
	    WaitForSingleObject (h, 10000);
	  ForceCloseHandle1 (h, hwait_sig);

	  /* Exiting thread.  Cleanup.  Don't set to inactive if a child has been
	     execed with the same pid. */
	  if (!myself->dwProcessId || myself->dwProcessId == GetCurrentProcessId ())
	    myself->process_state &= ~PID_ACTIVE;
	  else
	    sip_printf ("Did not clear PID_ACTIVE since %d != %d",
			myself->dwProcessId, GetCurrentProcessId ());

	  /* In case of a sigsuspend */
	  SetEvent (signal_arrived);

	  if (GetCurrentThreadId () != sigtid)
	    {
	      ForceCloseHandle (sigcomplete_main);
	      ForceCloseHandle (sigcomplete_nonmain);
	      ForceCloseHandle (sigcatch_main);
	      ForceCloseHandle (sigcatch_nonmain);
	      ForceCloseHandle (sigcatch_nosync);
	    }
	}
      sip_printf ("done");
    }

  /* Set this so that subsequent tests will succeed. */
  if (!myself->dwProcessId)
    myself->dwProcessId = GetCurrentProcessId ();

  return;
}

/* Send a signal to another process by raising its signal semaphore.
 * If pinfo *p == NULL, send to the current process.
 * If sending to this process, wait for notification that a signal has
 * completed before returning.
 */
int __stdcall
sig_send (pinfo *p, int sig, DWORD ebp)
{
  int rc = 1;
  DWORD tid = GetCurrentThreadId ();
  BOOL its_me;
  HANDLE thiscatch = NULL;
  HANDLE thiscomplete = NULL;
  BOOL wait_for_completion;
  sigframe thisframe;

  if (p == myself_nowait_nonmain)
    p = (tid == mainthread.id) ? myself : myself_nowait;
  if (!(its_me = (p == NULL || p == myself || p == myself_nowait)))
    wait_for_completion = FALSE;
  else
    {
      if (no_signals_available ())
	goto out;		// Either exiting or not yet initializing
      wait_for_me ();
      wait_for_completion = p != myself_nowait;
      p = myself;
    }

  /* It is possible that the process is not yet ready to receive messages
   * or that it has exited.  Detect this.
   */
  if (!proc_can_be_signalled (p))	/* Is the process accepting messages? */
    {
      sip_printf ("invalid pid %d(%x), signal %d",
		  p->pid, p->process_state, sig);
      set_errno (ESRCH);
      goto out;
    }

  sip_printf ("pid %d, signal %d, its_me %d", p->pid, sig, its_me);

  if (its_me)
    {
      if (!wait_for_completion)
	thiscatch = sigcatch_nosync;
      else if (tid != mainthread.id)
	{
	  thiscatch = sigcatch_nonmain;
	  thiscomplete = sigcomplete_nonmain;
	}
      else
	{
	  thiscatch = sigcatch_main;
	  thiscomplete = sigcomplete_main;
	  thisframe.set (mainthread, 1, ebp);
	}
    }
  else if (!(thiscatch = getsem (p, "sigcatch", 0, 0)))
    goto out;		  // Couldn't get the semaphore.  getsem issued
			  //  an error, if appropriate.

#if WHEN_MULTI_THREAD_SIGNALS_WORK
  signal_dispatch *sd;
  sd = signal_dispatch_storage.get ();
  if (sd == NULL)
    sd = signal_dispatch_storage.create ();
#endif

  /* Increment the sigtodo array to signify which signal to assert.
   */
  (void) InterlockedIncrement (p->getsigtodo(sig));

  /* Notify the process that a signal has arrived.
   */
  SetLastError (0);

#if 0
  int prio;
  prio = GetThreadPriority (GetCurrentThread ());
  (void) SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);
#endif

  if (!ReleaseSemaphore (thiscatch, 1, NULL) && (int) GetLastError () > 0)
    {
sigproc_printf ("ReleaseSemaphore failed, %E");
      /* Couldn't signal the semaphore.  This probably means that the
       * process is exiting.
       */
      if (!its_me)
	ForceCloseHandle (thiscatch);
      else
	{
	  if (no_signals_available ())
	    sip_printf ("I'm going away now");
	  else if ((int) GetLastError () == -1)
	    rc = WaitForSingleObject (thiscomplete, 500);
	  else
	    system_printf ("error sending signal %d to pid %d, semaphore %p, %E",
			  sig, p->pid, thiscatch);
	}
      goto out;
    }
sigproc_printf ("ReleaseSemaphore succeeded");

  /* No need to wait for signal completion unless this was a signal to
   * this process.
   *
   * If it was a signal to this process, wait for a dispatched signal.
   * Otherwise just wait for the wait_sig to signal that it has finished
   * processing the signal.
   */
  if (!wait_for_completion)
    {
      rc = WAIT_OBJECT_0;
      sip_printf ("Not waiting for sigcomplete.  its_me %d sig %d", its_me, sig);
      if (!its_me)
	ForceCloseHandle (thiscatch);
    }
  else
    {
      sip_printf ("Waiting for thiscomplete %p", thiscomplete);

      SetLastError (0);
      rc = WaitForSingleObject (thiscomplete, WSSC);
      /* Check for strangeness due to this thread being redirected by the
	 signal handler.  Sometimes a WAIT_TIMEOUT will occur when the
	 thread hasn't really timed out.  So, check again.
	 FIXME: This isn't foolproof. */
      if (rc != WAIT_OBJECT_0 &&
	  WaitForSingleObject (thiscomplete, 0) == WAIT_OBJECT_0)
	rc = WAIT_OBJECT_0;
    }

#if 0
  SetThreadPriority (GetCurrentThread (), prio);
#endif

  if (rc == WAIT_OBJECT_0)
    rc = 0;		// Successful exit
  else
    {
      /* It's an error unless sig_loop_wait == 0 (the process is exiting). */
      if (!no_signals_available ())
	system_printf ("wait for sig_complete event failed, sig %d, rc %d, %E",
		      sig, rc);
      set_errno (ENOSYS);
      rc = -1;
    }

out:
  sip_printf ("returning %d from sending signal %d", rc, sig);
  return rc;
}

/* Set pending signal from the sigtodo array
 */
void __stdcall
sig_set_pending (int sig)
{
  (void) InterlockedIncrement (myself->getsigtodo(sig));
  return;
}

/* Initialize the wait_subproc thread.
 * Called from fork() or spawn() to initialize the handling of subprocesses.
 */
void __stdcall
subproc_init (void)
{
  if (hwait_subproc)
    return;

  /* A "wakeup" handle which can be toggled to make wait_subproc reexamine
   * the hchildren array.
   */
  events[0] = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
  if (!(hwait_subproc = makethread (wait_subproc, NULL, 0, "+proc")))
    system_printf ("cannot create wait_subproc thread, %E");
  ProtectHandle (events[0]);
  ProtectHandle (hwait_subproc);
  sip_printf ("started wait_subproc thread %p", hwait_subproc);
}

/* Initialize some of the memory block passed to child processes
   by fork/spawn/exec. */

void __stdcall
init_child_info (DWORD chtype, child_info *ch, int pid, HANDLE subproc_ready)
{
  subproc_init ();
  memset (ch, 0, sizeof *ch);
  ch->cb = sizeof *ch;
  ch->type = chtype;
  ch->cygpid = pid;
  ch->shared_h = cygwin_shared_h;
  ch->console_h = console_shared_h;
  ch->subproc_ready = subproc_ready;
  if (chtype != PROC_EXEC || !parent_alive)
    ch->parent_alive = hwait_subproc;
  else if (parent_alive)
    DuplicateHandle (hMainProc, parent_alive, hMainProc, &ch->parent_alive,
		     0, 1, DUPLICATE_SAME_ACCESS);
}

/* Check the state of all of our children to see if any are stopped or
 * terminated.
 */
static int __stdcall
checkstate (waitq *w)
{
  int i, x, potential_match = 0;
  pinfo *child;

  sip_printf ("nchildren %d, nzombies %d", nchildren, nzombies);

  /* Check already dead processes first to see if they match the criteria
   * given in w->next.
   */
  for (i = 0; i < nzombies; i++)
    if ((x = stopped_or_terminated (w, child = zombies[i])) < 0)
      potential_match = -1;
    else if (x > 0)
      {
	remove_zombie (i);
	potential_match = 1;
	goto out;
      }

  sip_printf ("checking alive children");

  /* No dead terminated children matched.  Check for stopped children. */
  for (i = 0; i < nchildren; i++)
    if ((x = stopped_or_terminated (w, pchildren[i])) < 0)
      potential_match = -1;
    else if (x > 0)
      {
	potential_match = 1;
	break;
      }

out:
  sip_printf ("returning %d", potential_match);
  return potential_match;
}

/* Get or create a process specific semaphore used in message passing.
 */
static HANDLE __stdcall
getsem (pinfo *p, const char *str, int init, int max)
{
  HANDLE h;

  if (p != NULL)
    {
      if (!proc_can_be_signalled (p))
	{
	  set_errno (ESRCH);
	  return NULL;
	}
      int wait = 10000;
      sip_printf ("pid %d, ppid %d, wait %d, initializing %x", p->pid, p->ppid, wait,
		  ISSTATE (p, PID_INITIALIZING));
      for (int i = 0; ISSTATE (p, PID_INITIALIZING) && i < wait; i++)
	Sleep (1);
    }

  SetLastError (0);
  if (p == NULL)
    {
      char sa_buf[1024];

      DWORD winpid = GetCurrentProcessId ();
      h = CreateSemaphore (allow_ntsec ? sec_user (sa_buf) : &sec_none_nih,
			   init, max, str = shared_name (str, winpid));
      p = myself;
    }
  else
    {
      h = OpenSemaphore (SEMAPHORE_ALL_ACCESS, FALSE,
			 str = shared_name (str, p->dwProcessId));

      if (h == NULL)
	{
	  if (GetLastError () == ERROR_FILE_NOT_FOUND && !proc_exists (p))
	    set_errno (ESRCH);
	  else
	    set_errno (EPERM);
	  return NULL;
	}
    }

  if (!h)
    {
      system_printf ("can't %s %s, %E", p ? "open" : "create", str);
      set_errno (ESRCH);
    }
  return h;
}

/* Get the sync_proc_subproc muto to control access to
 * children, zombie arrays.
 * Attempt to handle case where process is exiting as we try to grab
 * the mutex.
 */
static __inline__ BOOL
get_proc_lock (DWORD what, DWORD val)
{
  Static int lastwhat = -1;
  if (!sync_proc_subproc)
    return FALSE;
  if (sync_proc_subproc->acquire (WPSP))
    {
      lastwhat = what;
      return TRUE;
    }
  if (!sync_proc_subproc)
    return FALSE;
  system_printf ("Couldn't aquire sync_proc_subproc for(%d,%d), %E, last %d",
		  what, val, lastwhat);
  return TRUE;
}

/* Remove a child from pchildren/hchildren by swapping it with the
 * last child in the list.
 */
static void __stdcall
remove_child (int ci)
{
  sip_printf ("removing [%d], pid %d, handle %p, nchildren %d",
	      ci, pchildren[ci]->pid, hchildren[ci], nchildren);
  if (ci < --nchildren)
    {
      pchildren[ci] = pchildren[nchildren];
      hchildren[ci] = hchildren[nchildren];
    }

  return;
}

/* Remove a zombie from zombies by swapping it with the last child in the list.
 */
static void __stdcall
remove_zombie (int ci)
{
  sip_printf ("removing %d, pid %d, nzombies %d", ci, zombies[ci]->pid,
	      nzombies);
  if (ci < --nzombies)
    zombies[ci] = zombies[nzombies];

  return;
}

/* Check status of child process vs. waitq member.
 *
 * parent_w is the pointer to the parent of the waitq member in question.
 * child is the subprocess being considered.
 *
 * Returns
 *   1 if stopped or terminated child matches parent_w->next criteria
 *  -1 if a non-stopped/terminated child matches parent_w->next criteria
 *   0 if child does not match parent_w->next criteria
 */
static int __stdcall
stopped_or_terminated (waitq *parent_w, pinfo *child)
{
  int potential_match;
  waitq *w = parent_w->next;

  sip_printf ("considering pid %d", child->pid);
  if (w->pid == -1)
    potential_match = 1;
  else if (w->pid == 0)
    potential_match = child->pgid == myself->pgid;
  else if (w->pid < 0)
    potential_match = child->pgid == -w->pid;
  else
    potential_match = (w->pid == child->pid);

  if (!potential_match)
    return 0;

  BOOL terminated;

  if ((terminated = child->process_state == PID_ZOMBIE) ||
      (w->options & WUNTRACED) && child->stopsig)
    {
      parent_w->next = w->next;	/* successful wait.  remove from wait queue */
      w->pid = child->pid;

      if (!terminated)
	{
	  sip_printf ("stopped child");
	  w->status = (child->stopsig << 8) | 0x7f;
	  child->stopsig = 0;
	}
      else
	{
	  DWORD status;
	  if (!GetExitCodeProcess (child->hProcess, &status))
	    status = 0xffff;
	  if (status & EXIT_SIGNAL)
	    w->status = (status >> 8) & 0xff;	/* exited due to signal */
	  else
	    w->status = (status & 0xff) << 8;	/* exited via "exit ()" */

	  add_rusage (&myself->rusage_children, &child->rusage_children);
	  add_rusage (&myself->rusage_children, &child->rusage_self);

	  if (w->rusage)
	    {
	      add_rusage ((struct rusage *) w->rusage, &child->rusage_children);
	      add_rusage ((struct rusage *) w->rusage, &child->rusage_self);
	    }
	  ForceCloseHandle1 (child->hProcess, childhProc);
	  child->hProcess = NULL;
	  child->process_state = PID_NOT_IN_USE;	/* a reaped child */
	}

      if (!SetEvent (w->ev))	/* wake up wait4 () immediately */
	system_printf ("couldn't wake up wait event %p, %E", w->ev);
      return 1;
    }

  return -potential_match;
}

/* Process signals by waiting for a semaphore to become signaled.
 * Then scan an in-memory array representing queued signals.
 * Executes in a separate thread.
 *
 * Signals sent from this process are sent a completion signal so
 * that returns from kill/raise do not occur until the signal has
 * has been handled, as per POSIX.
 */
static DWORD WINAPI
wait_sig (VOID *)
{
  /* Initialization */
  (void) SetThreadPriority (hwait_sig, WAIT_SIG_PRIORITY);

  /* sigcatch_nosync       - semaphore incremented by sig_dispatch_pending and
   *			     by foreign processes to force an examination of
   *			     the sigtodo array.
   * sigcatch_main	   - ditto for local main thread.
   * sigcatch_nonmain      - ditto for local non-main threads.
   *
   * sigcomplete_main	   - event used to signal main thread on signal
   *			     completion
   * sigcomplete_nonmain   - semaphore signaled for non-main thread on signal
   *			     completion
   */
  sigcatch_nosync = getsem (NULL, "sigcatch", 0, MAXLONG);
  sigcatch_nonmain = CreateSemaphore (&sec_none_nih, 0, MAXLONG, NULL);
  sigcatch_main = CreateSemaphore (&sec_none_nih, 0, MAXLONG, NULL);
  sigcomplete_nonmain = CreateSemaphore (&sec_none_nih, 0, MAXLONG, NULL);
  sigcomplete_main = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
  sigproc_printf ("sigcatch_nonmain %p", sigcatch_nonmain);

  /* Setting dwProcessId flags that this process is now capable of receiving
   * signals.  Prior to this, dwProcessId was set to the windows pid of
   * of the original windows process which spawned us unless this was a
   * "toplevel" process.
   */
  myself->dwProcessId = GetCurrentProcessId ();
  myself->process_state |= PID_ACTIVE;
  myself->process_state &= ~PID_INITIALIZING;

  ProtectHandle (sigcatch_nosync);
  ProtectHandle (sigcatch_nonmain);
  ProtectHandle (sigcatch_main);
  ProtectHandle (sigcomplete_nonmain);
  ProtectHandle (sigcomplete_main);

  /* If we've been execed, then there is still a stub left in the previous
   * windows process waiting to see if it's started a cygwin process or not.
   * Signalling subproc_ready indicates that we are a cygwin process.
   */
  if (child_proc_info && child_proc_info->type == PROC_EXEC)
    {
      debug_printf ("subproc_ready %p", child_proc_info->subproc_ready);
      if (!SetEvent (child_proc_info->subproc_ready))
	system_printf ("SetEvent (subproc_ready) failed, %E");
      ForceCloseHandle (child_proc_info->subproc_ready);
    }

  SetEvent (wait_sig_inited);
  sigtid = GetCurrentThreadId ();

  /* If we got something like a SIGINT while we were initializing, the
     signal thread should be waiting for this event.  This signals the
     thread that it's ok to send the signal since the wait_sig thread
     is now active. */
  extern HANDLE console_handler_thread_waiter;
  SetEvent (console_handler_thread_waiter);

  HANDLE catchem[] = {sigcatch_main, sigcatch_nonmain, sigcatch_nosync};
  sigproc_printf ("Ready.  dwProcessid %d", myself->dwProcessId);
  for (;;)
    {
      DWORD rc = WaitForMultipleObjects (3, catchem, FALSE, sig_loop_wait);

      /* sigproc_terminate sets sig_loop_wait to zero to indicate that
       * this thread should terminate.
       */
      if (rc == WAIT_TIMEOUT)
	if (!sig_loop_wait)
	  break;			// Exiting
	else
	  continue;

      if (rc == WAIT_FAILED)
	{
	  if (sig_loop_wait != 0)
	    system_printf ("WFMO failed, %E");
	  break;
	}

      rc -= WAIT_OBJECT_0;
      int dispatched = FALSE;
      sip_printf ("awake");
      /* A sigcatch semaphore has been signaled.  Scan the sigtodo
       * array looking for any unprocessed signals.
       */
      pending_signals = 0;
      int saw_sigchld = 0;
      int dispatched_sigchld = 0;
      for (int sig = -__SIGOFFSET; sig < NSIG; sig++)
	{
	  while (InterlockedDecrement (myself->getsigtodo(sig)) >= 0)
	    {
	      if (sig == SIGCHLD)
		saw_sigchld = 1;
	      if (sig > 0 && sig != SIGCONT && sig != SIGKILL && sig != SIGSTOP &&
		  (sigismember (& myself->getsigmask (), sig) ||
		   myself->process_state & PID_STOPPED))
		{
		  sip_printf ("sig %d blocked", sig);
		  break;
		}

	      /* Found a signal to process */
	      sip_printf ("processing signal %d", sig);
	      switch (sig)
		{
		case __SIGFLUSH:
		  /* just forcing the loop */
		  break;

		/* Internal signal to force a flush of strace data to disk. */
		case __SIGSTRACE:
		  // proc_strace ();	// Dump cached strace.prntf stuff.
		  break;

		/* Signalled from a child process that it has stopped */
		case __SIGCHILDSTOPPED:
		  sip_printf ("Received child stopped notification");
		  dispatched |= sig_handle (SIGCHLD);
		  if (proc_subproc (PROC_CHILDSTOPPED, 0))
		    dispatched |= 1;
		  break;

		/* A normal UNIX signal */
		default:
		  sip_printf ("Got signal %d", sig);
		  int wasdispatched = sig_handle (sig);
		  dispatched |= wasdispatched;
		  if (sig == SIGCHLD && wasdispatched)
		    dispatched_sigchld = 1;
		  goto nextsig;
		}
	    }
	  /* Decremented too far. */
	  if (InterlockedIncrement (myself->getsigtodo(sig)) > 0)
	    pending_signals = 1;
	nextsig:
	  continue;
	}

      if (saw_sigchld && !dispatched_sigchld)
	proc_subproc (PROC_CLEARWAIT, 0);
      /* Signal completion of signal handling depending on which semaphore
       * woke up the WaitForMultipleObjects above.
       */
      switch (rc)
	{
	case 0:
	  SetEvent (sigcomplete_main);
	  break;
	case 1:
	  ReleaseSemaphore (sigcomplete_nonmain, 1, NULL);
	  break;
	default:
	  /* Signal from another process.  No need to synchronize. */
	  break;
	}

      if (dispatched < 0)
	pending_signals = 1;
      sip_printf ("looping");
    }

  sip_printf ("done");
  return 0;
}

/* Wait for subprocesses to terminate. Executes in a separate thread. */
static DWORD WINAPI
wait_subproc (VOID *)
{
  sip_printf ("starting");
  int errloop = 0;

  for (;;)
    {
      DWORD rc = WaitForMultipleObjects (nchildren + 1, events, FALSE,
					 proc_loop_wait);
      if (rc == WAIT_TIMEOUT)
	if (!proc_loop_wait)
	  break;			// Exiting
	else
	  continue;

      if (rc == WAIT_FAILED)
	{
	  if (!proc_loop_wait)
	    break;

	  /* It's ok to get an ERROR_INVALID_HANDLE since another thread may have
	     closed a handle in the children[] array.  So, we try looping a couple
	     of times to stabilize. FIXME - this is not foolproof.  Probably, this
	     thread should be responsible for closing the children. */
	  if (++errloop < 10 && GetLastError () == ERROR_INVALID_HANDLE)
	    continue;

	  system_printf ("wait failed. nchildren %d, wait %d, %E",
			nchildren, proc_loop_wait);

	  for (int i = 0; i < nchildren + 1; i++)
	    if ((rc = WaitForSingleObject (events[i], 0)) == WAIT_OBJECT_0 ||
		rc == WAIT_TIMEOUT)
	      continue;
	    else
	      system_printf ("event[%d] %p, %E", i, events[0]);
	  break;
	}

      errloop = 0;
      rc -= WAIT_OBJECT_0;
      if (rc-- != 0)
	(void)proc_subproc (PROC_CHILDTERMINATED, rc);
      sip_printf ("looping");
    }

  ForceCloseHandle (events[0]);
  events[0] = NULL;
  sip_printf ("done");
  return 0;
}

extern "C" {
/* Provide a stack frame when calling WaitFor* functions */

#undef WaitForSingleObject

DWORD __stdcall
WFSO (HANDLE hHandle, DWORD dwMilliseconds)
{
  DWORD ret;
  sigframe thisframe (mainthread);
  ret = WaitForSingleObject (hHandle, dwMilliseconds);
  return ret;
}

#undef WaitForMultipleObjects

DWORD __stdcall
WFMO (DWORD nCount, CONST HANDLE *lpHandles, BOOL fWaitAll, DWORD dwMilliseconds)
{
  DWORD ret;
  sigframe thisframe (mainthread);
  ret = WaitForMultipleObjects (nCount, lpHandles, fWaitAll, dwMilliseconds);
  return ret;
}
}
