/* sigproc.cc: inter/intra signal and sub process handler

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.

   Written by Christopher Faylor

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include <assert.h>
#include <sys/signal.h>
#include "cygerrno.h"
#include "sync.h"
#include "pinfo.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info_magic.h"
#include "shared_info.h"
#include "cygthread.h"
#include "cygtls.h"
#include "sigproc.h"
#include "perthread.h"
#include "exceptions.h"

/*
 * Convenience defines
 */
#define WSSC		  60000	// Wait for signal completion
#define WPSP		  40000	// Wait for proc_subproc mutex

#define PSIZE 63		// Number of processes

#define wake_wait_subproc() SetEvent (events[0])

#define no_signals_available() (!hwait_sig || (myself->sendsig == INVALID_HANDLE_VALUE) || exit_state)

#define NZOMBIES	256

struct sigelem
{
  int sig;
  int pid;
  _threadinfo *tls;
  class sigelem *next;
  friend class pending_signals;
  friend int __stdcall sig_dispatch_pending ();
};

class pending_signals
{
  sigelem sigs[NSIG + 1];
  sigelem start;
  sigelem *end;
  sigelem *prev;
  sigelem *curr;
  int empty;
public:
  void reset () {curr = &start; prev = &start;}
  void add (int sig, int pid, _threadinfo *tls);
  void del ();
  sigelem *next ();
  sigelem *save () const {return curr;}
  void restore (sigelem *saved) {curr = saved;}
  friend int __stdcall sig_dispatch_pending ();
};

struct sigpacket
{
  int sig;
  pid_t pid;
  HANDLE wakeup;
  sigset_t *mask;
  _threadinfo *tls;
};

static pending_signals sigqueue;

struct sigaction *global_sigs;

void __stdcall
sigalloc ()
{
  cygheap->sigs = global_sigs =
    (struct sigaction *) ccalloc (HEAP_SIGS, NSIG, sizeof (struct sigaction));
}

void __stdcall
signal_fixup_after_exec ()
{
  global_sigs = cygheap->sigs;
  /* Set up child's signal handlers */
  for (int i = 0; i < NSIG; i++)
    {
      global_sigs[i].sa_mask = 0;
      if (global_sigs[i].sa_handler != SIG_IGN)
	global_sigs[i].sa_handler = SIG_DFL;
    }
}

/*
 * Global variables
 */
const char *__sp_fn ;
int __sp_ln;

char NO_COPY myself_nowait_dummy[1] = {'0'};// Flag to sig_send that signal goes to
					//  current process but no wait is required
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

Static DWORD proc_loop_wait = 1000;	// Wait for subprocesses to exit

Static HANDLE sigcomplete_main;		// Event signaled when a signal has
					//  finished processing for the main
					//  thread
HANDLE NO_COPY sigCONT;			// Used to "STOP" a process
Static cygthread *hwait_sig;		// Handle of wait_sig thread
Static cygthread *hwait_subproc;	// Handle of sig_subproc thread

Static HANDLE wait_sig_inited;		// Control synchronization of
					//  message queue startup

/* Used by WaitForMultipleObjects.  These are handles to child processes.
 */
Static HANDLE events[PSIZE + 1];	  // All my children's handles++
#define hchildren (events + 1)		// Where the children handles begin
Static char cpchildren[PSIZE * sizeof (pinfo)];		// All my children info
Static int nchildren;			// Number of active children
Static char czombies[(NZOMBIES + 1) * sizeof (pinfo)];		// All my deceased children info
Static int nzombies;			// Number of deceased children

#define pchildren ((pinfo *) cpchildren)
#define zombies ((pinfo *) czombies)

Static waitq waitq_head = {0, 0, 0, 0, 0, 0, 0};// Start of queue for wait'ing threads
Static waitq waitq_main;		// Storage for main thread

muto NO_COPY *sync_proc_subproc = NULL;	// Control access to subproc stuff

DWORD NO_COPY sigtid = 0;		// ID of the signal thread

/* Functions
 */
static int __stdcall checkstate (waitq *) __attribute__ ((regparm (1)));
static __inline__ bool get_proc_lock (DWORD, DWORD);
static void __stdcall remove_zombie (int);
static DWORD WINAPI wait_sig (VOID *arg);
static int __stdcall stopped_or_terminated (waitq *, _pinfo *);
static DWORD WINAPI wait_subproc (VOID *);

/* Determine if the parent process is alive.
 */

bool __stdcall
my_parent_is_alive ()
{
  bool res;
  if (!myself->ppid_handle)
    {
      debug_printf ("No myself->ppid_handle");
      res = false;
    }
  else
    for (int i = 0; i < 2; i++)
      switch (res = WaitForSingleObject (myself->ppid_handle, 0))
	{
	  case WAIT_OBJECT_0:
	    debug_printf ("parent dead.");
	    res = false;
	    goto out;
	  case WAIT_TIMEOUT:
	    debug_printf ("parent still alive");
	    res = true;
	    goto out;
	  case WAIT_FAILED:
	    DWORD werr = GetLastError ();
	    if (werr == ERROR_INVALID_HANDLE && i == 0)
	      continue;
	    system_printf ("WFSO for myself->ppid_handle(%p) failed, error %d",
			   myself->ppid_handle, werr);
	    res = false;
	    goto out;
	}
out:
  return res;
}

void __stdcall
wait_for_sigthread ()
{
  sigproc_printf ("wait_sig_inited %p", wait_sig_inited);
  HANDLE hsig_inited = wait_sig_inited;
  (void) WaitForSingleObject (hsig_inited, INFINITE);
  wait_sig_inited = NULL;
  (void) ForceCloseHandle1 (hsig_inited, wait_sig_inited);
}

/* Get the sync_proc_subproc muto to control access to
 * children, zombie arrays.
 * Attempt to handle case where process is exiting as we try to grab
 * the mutex.
 */
static bool
get_proc_lock (DWORD what, DWORD val)
{
  Static int lastwhat = -1;
  if (!sync_proc_subproc)
    {
      sigproc_printf ("sync_proc_subproc is NULL (1)");
      return false;
    }
  if (sync_proc_subproc->acquire (WPSP))
    {
      lastwhat = what;
      return true;
    }
  if (!sync_proc_subproc)
    {
      sigproc_printf ("sync_proc_subproc is NULL (2)");
      return false;
    }
  system_printf ("Couldn't aquire sync_proc_subproc for(%d,%d), %E, last %d",
		  what, val, lastwhat);
  return true;
}

static bool __stdcall
proc_can_be_signalled (_pinfo *p)
{
  if (p->sendsig == INVALID_HANDLE_VALUE)
    {
      set_errno (EPERM);
      return false;
    }

  if (p == myself_nowait || p == myself)
    return hwait_sig;

  if (ISSTATE (p, PID_INITIALIZING) ||
      (((p)->process_state & (PID_ACTIVE | PID_IN_USE)) ==
       (PID_ACTIVE | PID_IN_USE)))
    return true;

  set_errno (ESRCH);
  return false;
}

bool __stdcall
pid_exists (pid_t pid)
{
  pinfo p (pid);
  return proc_exists (p);
}

/* Test to determine if a process really exists and is processing signals.
 */
bool __stdcall
proc_exists (_pinfo *p)
{
  return p && !(p->process_state & (PID_EXITED | PID_ZOMBIE));
}

/* Return 1 if this is one of our children, zero otherwise.
   FIXME: This really should be integrated with the rest of the proc_subproc
   testing.  Scanning these lists twice is inefficient. */
int __stdcall
mychild (int pid)
{
  for (int i = 0; i < nchildren; i++)
    if (pchildren[i]->pid == pid)
      return 1;
  for (int i = 0; i < nzombies; i++)
    if (zombies[i]->pid == pid)
      return 1;
  return 0;
}

/* Handle all subprocess requests
 */
#define vchild (*((pinfo *) val))
int __stdcall
proc_subproc (DWORD what, DWORD val)
{
  int rc = 1;
  int potential_match;
  _pinfo *child;
  int clearing;
  waitq *w;
  int thiszombie;

#define wval	 ((waitq *) val)

  sigproc_printf ("args: %x, %d", what, val);

  if (!get_proc_lock (what, val))	// Serialize access to this function
    {
      system_printf ("couldn't get proc lock. what %d, val %d", what, val);
      goto out1;
    }

  switch (what)
    {
    /* Add a new subprocess to the children arrays.
     * (usually called from the main thread)
     */
    case PROC_ADDCHILD:
      if (nchildren >= PSIZE - 1)
	{
	  rc = 0;
	  break;
	}
      pchildren[nchildren] = vchild;
      hchildren[nchildren] = vchild->hProcess;
      if (!DuplicateHandle (hMainProc, vchild->hProcess, hMainProc, &vchild->pid_handle,
			    0, 0, DUPLICATE_SAME_ACCESS))
	system_printf ("Couldn't duplicate child handle for pid %d, %E", vchild->pid);
      ProtectHandle1 (vchild->pid_handle, pid_handle);

      if (!DuplicateHandle (hMainProc, hMainProc, vchild->hProcess, &vchild->ppid_handle,
			    SYNCHRONIZE | PROCESS_DUP_HANDLE, TRUE, 0))
	system_printf ("Couldn't duplicate my handle<%p> for pid %d, %E", hMainProc, vchild->pid);
      vchild->ppid = myself->pid;
      vchild->uid = myself->uid;
      vchild->gid = myself->gid;
      vchild->pgid = myself->pgid;
      vchild->sid = myself->sid;
      vchild->ctty = myself->ctty;
      vchild->process_state |= PID_INITIALIZING | (myself->process_state & PID_USETTY);

      sigproc_printf ("added pid %d to wait list, slot %d, winpid %p, handle %p",
		  vchild->pid, nchildren, vchild->dwProcessId,
		  vchild->hProcess);
      nchildren++;

      wake_wait_subproc ();
      break;

    /* A child process had terminated.
       Possibly this is just due to an exec().  Cygwin implements an exec()
       as a "handoff" from one windows process to another.  If child->hProcess
       is different from what is recorded in hchildren, then this is an exec().
       Otherwise this is a normal child termination event.
       (called from wait_subproc thread) */
    case PROC_CHILDTERMINATED:
      if (hchildren[val] != pchildren[val]->hProcess)
	{
	  sigproc_printf ("pid %d[%d], reparented old hProcess %p, new %p",
			  pchildren[val]->pid, val, hchildren[val], pchildren[val]->hProcess);
	  HANDLE h = hchildren[val];
	  hchildren[val] = pchildren[val]->hProcess; /* Filled out by child */
	  sync_proc_subproc->release ();	// Release the lock ASAP
	  ForceCloseHandle1 (h, childhProc);
	  ProtectHandle1 (pchildren[val]->hProcess, childhProc);
	  rc = 0;
	  goto out;			// This was an exec()
	}

      sigproc_printf ("pid %d[%d] terminated, handle %p, nchildren %d, nzombies %d",
		  pchildren[val]->pid, val, hchildren[val], nchildren, nzombies);

      thiszombie = nzombies;
      zombies[nzombies] = pchildren[val];	// Add to zombie array
      zombies[nzombies++]->process_state = PID_ZOMBIE;// Walking dead

      sigproc_printf ("zombifying [%d], pid %d, handle %p, nchildren %d",
		      val, pchildren[val]->pid, hchildren[val], nchildren);
      if ((int) val < --nchildren)
	{
	  hchildren[val] = hchildren[nchildren];
	  pchildren[val] = pchildren[nchildren];
	}

      /* See if we should care about the this terminated process.  If we've
	 filled up our table or if we're ignoring SIGCHLD, then we immediately
	 remove the process and move on. Otherwise, this process becomes a zombie
	 which must be reaped by a wait() call.  FIXME:  This is a very inelegant
	 way to deal with this and could lead to process hangs.  */
      if (nzombies >= NZOMBIES)
	{
	  sigproc_printf ("zombie table overflow %d", thiszombie);
	  remove_zombie (thiszombie);
	}

      /* Don't scan the wait queue yet.  Caller will send SIGCHLD to this process.
	 This will cause an eventual scan of waiters. */
      break;

    /* Handle a wait4() operation.  Allocates an event for the calling
     * thread which is signaled when the appropriate pid exits or stops.
     * (usually called from the main thread)
     */
    case PROC_WAIT:
      wval->ev = NULL;		// Don't know event flag yet

      if (wval->pid <= 0)
	child = NULL;		// Not looking for a specific pid
      else if (!mychild (wval->pid))
	goto out;		// invalid pid.  flag no such child

      wval->status = 0;		// Don't know status yet
      sigproc_printf ("wval->pid %d, wval->options %d", wval->pid, wval->options);

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
      w = waitq_head.next;
      waitq_head.next = wval;	/* Add at the beginning. */
      wval->next = w;		/* Link in rest of the list. */
      clearing = 0;
      goto scan_wait;

    /* Clear all waiting threads.  Called from exceptions.cc prior to
       the main thread's dispatch to a signal handler function.
       (called from wait_sig thread) */
    case PROC_CLEARWAIT:
      /* Clear all "wait"ing threads. */
      if (val)
	sigproc_printf ("clear waiting threads");
      else
	sigproc_printf ("looking for processes to reap");
      clearing = val;

    scan_wait:
      /* Scan the linked list of wait()ing threads.  If a wait's parameters
       * match this pid, then activate it.
       */
      for (w = &waitq_head; w->next != NULL; w = w->next)
	{
	  if ((potential_match = checkstate (w)) > 0)
	    sigproc_printf ("released waiting thread");
	  else if (!clearing && !(w->next->options & WNOHANG) && potential_match < 0)
	    sigproc_printf ("only found non-terminated children");
	  else if (potential_match <= 0)		// nothing matched
	    {
	      sigproc_printf ("waiting thread found no children");
	      HANDLE oldw = w->next->ev;
	      w->next->pid = 0;
	      if (clearing)
		w->next->status = -1;		/* flag that a signal was received */
	      else if (!potential_match || !(w->next->options & WNOHANG))
		w->next->ev = NULL;
	      if (!SetEvent (oldw))
		system_printf ("couldn't wake up wait event %p, %E", oldw);
	      w->next = w->next->next;
	    }
	  if (w->next == NULL)
	    break;
	}

      if (!clearing)
	sigproc_printf ("finished processing terminated/stopped child");
      else
	{
	  waitq_head.next = NULL;
	  sigproc_printf ("finished clearing");
	}

      if (global_sigs[SIGCHLD].sa_handler == (void *) SIG_IGN)
	while (nzombies)
	  remove_zombie (0);
      break;
  }

out:
  sync_proc_subproc->release ();	// Release the lock
out1:
  sigproc_printf ("returning %d", rc);
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
  sigproc_printf ("nchildren %d, nzombies %d", nchildren, nzombies);
  /* Signal processing is assumed to be blocked in this routine. */
  if (hwait_subproc)
    {
      proc_loop_wait = 0;	// Tell wait_subproc thread to exit
      sync_proc_subproc->acquire (WPSP);
      wake_wait_subproc ();	// Wake wait_subproc loop
      hwait_subproc = NULL;

      (void) proc_subproc (PROC_CLEARWAIT, 1);

      /* Clean out zombie processes from the pid list. */
      int i;
      for (i = 0; i < nzombies; i++)
	{
	  if (zombies[i]->hProcess)
	    {
	      ForceCloseHandle1 (zombies[i]->hProcess, childhProc);
	      ForceCloseHandle1 (zombies[i]->pid_handle, pid_handle);
	    }
	  zombies[i]->ppid = 1;
	  zombies[i]->process_state = PID_EXITED;	/* CGF FIXME - still needed? */
	  zombies[i].release ();	// FIXME: this breaks older gccs for some reason
	}

      /* Disassociate my subprocesses */
      for (i = 0; i < nchildren; i++)
	{
	  if (!pchildren[i]->hProcess)
	    sigproc_printf ("%d(%d) hProcess cleared already?", pchildren[i]->pid,
			pchildren[i]->dwProcessId);
	  else
	    {
	      ForceCloseHandle1 (pchildren[i]->hProcess, childhProc);
	      sigproc_printf ("%d(%d) closed child handle", pchildren[i]->pid,
			      pchildren[i]->dwProcessId);
	      pchildren[i]->ppid = 1;
	      if (pchildren[i]->pgid == myself->pid)
		pchildren[i]->process_state |= PID_ORPHANED;
	    }
	  pchildren[i].release ();
	}
      nchildren = nzombies = 0;
      sync_proc_subproc = NULL;
    }
  sigproc_printf ("leaving");
}

/* Clear pending signal */
void __stdcall
sig_clear (int target_sig)
{
  if (GetCurrentThreadId () != sigtid)
    sig_send (myself, -target_sig);
  else
    {
      sigelem *q;
      sigelem *save = sigqueue.save ();
      sigqueue.reset ();
      while ((q = sigqueue.next ()))
	if (q->sig == target_sig)
	  {
	    q->sig = __SIGDELETE;
	    break;
	  }
      sigqueue.restore (save);
    }
  return;
}

extern "C" int
sigpending (sigset_t *mask)
{
  sigset_t outset = (sigset_t) sig_send (myself, __SIGPENDING);
  if (outset == SIG_BAD_MASK)
    return -1;
  *mask = outset;
  return 0;
}

/* Force the wait_sig thread to wake up and scan for pending signals */
int __stdcall
sig_dispatch_pending ()
{
  if (exit_state || GetCurrentThreadId () == sigtid || !sigqueue.start.next)
    {
#ifdef DEBUGGING
      sigproc_printf ("exit_state %d, GetCurrentThreadId () %p, sigtid %p, sigqueue.start.next %p",
		      exit_state, GetCurrentThreadId (), sigtid, sigqueue.start.next);
#endif
      return 0;
    }

#ifdef DEBUGGING
  sigproc_printf ("flushing");
#endif
  (void) sig_send (myself, __SIGFLUSH);
  return call_signal_handler_now ();
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
  wait_sig_inited = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
  ProtectHandle (wait_sig_inited);

  /* sync_proc_subproc is used by proc_subproc.  It serialises
   * access to the children and zombie arrays.
   */
  new_muto (sync_proc_subproc);

  /* local event signaled when main thread has been dispatched
     to a signal handler function. */
  signal_arrived = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
  ProtectHandle (signal_arrived);

  hwait_sig = new cygthread (wait_sig, cygself, "sig");
  hwait_sig->zap_h ();

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

  global_sigs[SIGSTOP].sa_flags = SA_RESTART | SA_NODEFER;
  sigproc_printf ("process/signal handling enabled(%x)", myself->process_state);
  return;
}

/* Called on process termination to terminate signal and process threads.
 */
void __stdcall
sigproc_terminate (void)
{
  extern HANDLE hExeced;
  hwait_sig = NULL;

  if (myself->sendsig == INVALID_HANDLE_VALUE)
    sigproc_printf ("sigproc handling not active");
  else
    {
      sigproc_printf ("entering");
				//  finished with anything it is doing
      ForceCloseHandle (sigcomplete_main);
      if (!hExeced)
	{
	  HANDLE sendsig = myself->sendsig;
	  myself->sendsig = INVALID_HANDLE_VALUE;
	  CloseHandle (sendsig);
	}
    }
  proc_terminate ();		// Terminate process handling thread

  return;
}

/* Send a signal to another process by raising its signal semaphore.
 * If pinfo *p == NULL, send to the current process.
 * If sending to this process, wait for notification that a signal has
 * completed before returning.
 */
int __stdcall
sig_send (_pinfo *p, int sig, void *tls)
{
  int rc = 1;
  bool its_me;
  HANDLE sendsig;
  sigpacket pack;

  bool wait_for_completion;
  // FIXMENOW: Avoid using main thread's completion event!
  if (!(its_me = (p == NULL || p == myself || p == myself_nowait)))
    wait_for_completion = false;
  else
    {
      if (no_signals_available ())
	{
	  sigproc_printf ("hwait_sig %p, myself->sendsig %p, exit_state %d",
			  hwait_sig, myself->sendsig, exit_state);
	  goto out;		// Either exiting or not yet initializing
	}
      if (wait_sig_inited)
	wait_for_sigthread ();
      wait_for_completion = p != myself_nowait && _my_tls.isinitialized ();
      p = myself;
    }

  /* It is possible that the process is not yet ready to receive messages
   * or that it has exited.  Detect this.
   */
  if (!proc_can_be_signalled (p))	/* Is the process accepting messages? */
    {
      sigproc_printf ("invalid pid %d(%x), signal %d",
		  p->pid, p->process_state, sig);
      goto out;
    }

  if (its_me)
    {
      sendsig = myself->sendsig;
      if (wait_for_completion)
	pack.wakeup = sigcomplete_main;
    }
  else
    {
      HANDLE hp = OpenProcess (PROCESS_DUP_HANDLE, false, p->dwProcessId);
      if (!hp)
	{
	  sigproc_printf ("OpenProcess failed, %E");
	  __seterrno ();
	  goto out;
	}
      for (int i = 0; !p->sendsig && i < 10000; i++)
	low_priority_sleep (0);
      if (!DuplicateHandle (hp, p->sendsig, hMainProc, &sendsig, false, 0,
			    DUPLICATE_SAME_ACCESS) || !sendsig)
	{
	  sigproc_printf ("DuplicateHandle failed, %E");
	  __seterrno ();
	  goto out;
	}
      CloseHandle (hp);
      pack.wakeup = NULL;
    }

  sigproc_printf ("sendsig %p, pid %d, signal %d, its_me %d", sendsig, p->pid,
		  sig, its_me);

  sigset_t pending;
  if (!its_me)
    pack.mask = NULL;
  else if (sig == __SIGPENDING)
    pack.mask = &pending;
  else if (sig == __SIGFLUSH || sig > 0)
    pack.mask = &myself->getsigmask ();
  else
    pack.mask = NULL;

  pack.sig = sig;
  pack.pid = myself->pid;
  pack.tls = (_threadinfo *) tls;
  DWORD nb;
  if (!WriteFile (sendsig, &pack, sizeof (pack), &nb, NULL) || nb != sizeof (pack))
    {
      /* Couldn't send to the pipe.  This probably means that the
         process is exiting.  */
      if (!its_me)
	{
	  sigproc_printf ("WriteFile for pipe %p failed, %E", sendsig);
	  __seterrno ();
	  ForceCloseHandle (sendsig);
	}
      else
	{
	  if (no_signals_available ())
	    sigproc_printf ("I'm going away now");
	  else
	    system_printf ("error sending signal %d to pid %d, pipe handle %p, %E",
			  sig, p->pid, sendsig);
	}
      goto out;
    }


  /* No need to wait for signal completion unless this was a signal to
     this process.

     If it was a signal to this process, wait for a dispatched signal.
     Otherwise just wait for the wait_sig to signal that it has finished
     processing the signal.  */
  if (wait_for_completion)
    {
      sigproc_printf ("Waiting for pack.wakeup %p", pack.wakeup);
      rc = WaitForSingleObject (pack.wakeup, WSSC);
    }
  else
    {
      rc = WAIT_OBJECT_0;
      sigproc_printf ("Not waiting for sigcomplete.  its_me %d signal %d", its_me, sig);
      if (!its_me)
	ForceCloseHandle (sendsig);
    }

  if (rc == WAIT_OBJECT_0)
    rc = 0;		// Successful exit
  else
    {
      if (!no_signals_available ())
	system_printf ("wait for sig_complete event failed, signal %d, rc %d, %E",
		      sig, rc);
      set_errno (ENOSYS);
      rc = -1;
    }

  if (wait_for_completion)
    call_signal_handler_now ();

out:
  if (sig != __SIGPENDING)
    /* nothing */;
  else if (!rc)
    rc = (int) pending;
  else
    rc = SIG_BAD_MASK;
  sigproc_printf ("returning %p from sending signal %d", rc, sig);
  return rc;
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
  hwait_subproc = new cygthread (wait_subproc, NULL, "proc");
  hwait_subproc->zap_h ();
  ProtectHandle (events[0]);
  sigproc_printf ("started wait_subproc thread");
}

/* Initialize some of the memory block passed to child processes
   by fork/spawn/exec. */

void __stdcall
init_child_info (DWORD chtype, child_info *ch, pid_t pid, HANDLE subproc_ready)
{
  memset (ch, 0, sizeof *ch);
  ch->cb = chtype == PROC_FORK ? sizeof (child_info_fork) : sizeof (child_info);
  ch->intro = PROC_MAGIC_GENERIC;
  ch->magic = CHILD_INFO_MAGIC;
  ch->type = chtype;
  ch->cygpid = pid;
  ch->subproc_ready = subproc_ready;
  ch->pppid_handle = myself->ppid_handle;
  ch->fhandler_union_cb = sizeof (fhandler_union);
  ch->user_h = cygwin_user_h;
}

/* Check the state of all of our children to see if any are stopped or
 * terminated.
 */
static int __stdcall
checkstate (waitq *parent_w)
{
  int potential_match = 0;

  sigproc_printf ("nchildren %d, nzombies %d", nchildren, nzombies);

  /* Check already dead processes first to see if they match the criteria
   * given in w->next.
   */
  for (int i = 0; i < nzombies; i++)
    switch (stopped_or_terminated (parent_w, zombies[i]))
      {
      case -1:
	potential_match = -1;
	break;
      case 1:
	remove_zombie (i);
	potential_match = 1;
	goto out;
      }

  sigproc_printf ("checking alive children");

  /* No dead terminated children matched.  Check for stopped children. */
  for (int i = 0; i < nchildren; i++)
    switch (stopped_or_terminated (parent_w, pchildren[i]))
      {
      case -1:
	potential_match = -1;
	break;
      case 1:
	potential_match = 1;
	goto out;
      }

out:
  sigproc_printf ("returning %d", potential_match);
  return potential_match;
}

/* Remove a zombie from zombies by swapping it with the last child in the list.
 */
static void __stdcall
remove_zombie (int ci)
{
  sigproc_printf ("removing %d, pid %d, nzombies %d", ci, zombies[ci]->pid,
		  nzombies);

  if (zombies[ci])
    {
      ForceCloseHandle1 (zombies[ci]->hProcess, childhProc);
      ForceCloseHandle1 (zombies[ci]->pid_handle, pid_handle);
      zombies[ci].release ();
    }

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
stopped_or_terminated (waitq *parent_w, _pinfo *child)
{
  int potential_match;
  waitq *w = parent_w->next;

  sigproc_printf ("considering pid %d", child->pid);
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

  bool terminated;

  if ((terminated = child->process_state == PID_ZOMBIE) ||
      ((w->options & WUNTRACED) && child->stopsig))
    {
      parent_w->next = w->next;	/* successful wait.  remove from wait queue */
      w->pid = child->pid;

      if (!terminated)
	{
	  sigproc_printf ("stopped child");
	  w->status = (child->stopsig << 8) | 0x7f;
	  child->stopsig = 0;
	}
      else /* Should only get here when child has been moved to the zombies array */
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
	}

      if (!SetEvent (w->ev))	/* wake up wait4 () immediately */
	system_printf ("couldn't wake up wait event %p, %E", w->ev);
      return 1;
    }

  return -potential_match;
}

static void
talktome ()
{
  winpids pids ((DWORD) PID_MAP_RW);
  for (unsigned i = 0; i < pids.npids; i++)
    if (pids[i]->hello_pid == myself->pid)
      if (!IsBadWritePtr (pids[i], sizeof (_pinfo)))
	pids[i]->commune_recv ();
}

/* Process signals by waiting for a semaphore to become signaled.
   Then scan an in-memory array representing queued signals.
   Executes in a separate thread.

   Signals sent from this process are sent a completion signal so
   that returns from kill/raise do not occur until the signal has
   has been handled, as per POSIX.  */

void
pending_signals::add (int sig, int pid, _threadinfo *tls)
{
  sigelem *se;
  for (se = start.next; se; se = se->next)
    if (se->sig == sig)
      return;
  while (sigs[empty].sig)
    if (++empty == NSIG)
      empty = 0;
  se = sigs + empty;
  se->sig = sig;
  se->next = NULL;
  se->tls = tls;
  se->pid = pid;
  if (end)
    end->next = se;
  end = se;
  if (!start.next)
    start.next = se;
  empty++;
}

void
pending_signals::del ()
{
  sigelem *next = curr->next;
  prev->next = next;
  curr->sig = 0;
#ifdef DEBUGGING
  curr->next = NULL;
#endif
  if (end == curr)
    end = prev;
  empty = curr - sigs;
  curr = next;
}

sigelem *
pending_signals::next ()
{
  sigelem *res;
  prev = curr;
  if (!curr || !(curr = curr->next))
    res = NULL;
  else
    res = curr;
  return res;
}

/* Process signals by waiting for signal data to arrive in a pipe.
   Set a completion event if one was specified. */
static DWORD WINAPI
wait_sig (VOID *self)
{
  HANDLE readsig;
  char sa_buf[1024];

  /* Initialization */
  (void) SetThreadPriority (GetCurrentThread (), WAIT_SIG_PRIORITY);

  /* sigcomplete_main	   - event used to signal main thread on signal
    			     completion */
  if (!CreatePipe (&readsig, &myself->sendsig, sec_user_nih (sa_buf), 0))
    api_fatal ("couldn't create signal pipe, %E");
  sigcomplete_main = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
  sigproc_printf ("sigcomplete_main %p", sigcomplete_main);
  sigCONT = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);

  /* Setting dwProcessId flags that this process is now capable of receiving
     signals.  Prior to this, dwProcessId was set to the windows pid of
     of the original windows process which spawned us unless this was a
     "toplevel" process.  */
  myself->dwProcessId = GetCurrentProcessId ();
  myself->process_state |= PID_ACTIVE;
  myself->process_state &= ~PID_INITIALIZING;

  ProtectHandle (sigcomplete_main);

  /* If we've been execed, then there is still a stub left in the previous
     windows process waiting to see if it's started a cygwin process or not.
     Signalling subproc_ready indicates that we are a cygwin process.  */
  if (child_proc_info && child_proc_info->type == PROC_EXEC)
    {
      debug_printf ("subproc_ready %p", child_proc_info->subproc_ready);
      if (!SetEvent (child_proc_info->subproc_ready))
	system_printf ("SetEvent (subproc_ready) failed, %E");
      ForceCloseHandle1 (child_proc_info->subproc_ready, subproc_ready);
      /* Initialize an "indirect" pid block so that if someone looks up this
	 process via its Windows PID it will be redirected to the appropriate
	 Cygwin PID shared memory block. */
      static pinfo NO_COPY myself_identity;
      myself_identity.init (cygwin_pid (myself->dwProcessId), PID_EXECED);
    }

  SetEvent (wait_sig_inited);
  sigtid = GetCurrentThreadId ();

  exception_list el;
  _my_tls.init_threadlist_exceptions (&el);
  debug_printf ("entering ReadFile loop, readsig %p, myself->sendsig %p",
		readsig, myself->sendsig);

  for (;;)
    {
      DWORD nb;
      sigpacket pack;
      if (!ReadFile (readsig, &pack, sizeof (pack), &nb, NULL))
	break;
      if (myself->sendsig == INVALID_HANDLE_VALUE)
	break;

      if (nb != sizeof (pack))
	{
	  system_printf ("short read from signal pipe: %d != %d", nb,
			 sizeof (pack));
	  continue;
	}

      if (!pack.sig)
	{
#ifdef DEBUGGING
	  system_printf ("zero signal?");
#endif
	  continue;
	}

      sigset_t dummy_mask;
      if (!pack.mask)
	{
	  dummy_mask = myself->getsigmask ();
	  pack.mask = &dummy_mask;
	}

      sigelem *q;
      switch (pack.sig)
	{
	case __SIGCOMMUNE:
	  talktome ();
	  break;
	case __SIGSTRACE:
	  strace.hello ();
	  break;
	case __SIGPENDING:
	  *pack.mask = 0;
	  unsigned bit;
	  sigqueue.reset ();
	  while ((q = sigqueue.next ()))
	    if (myself->getsigmask () & (bit = SIGTOMASK (q->sig)))
	      *pack.mask |= bit;
	  break;
	case __SIGFLUSH:
	  sigqueue.reset ();
	  while ((q = sigqueue.next ()))
	    if (q->sig == __SIGDELETE
		|| (sig_handle (q->sig, *pack.mask, q->pid, q->tls) > 0))
	      sigqueue.del ();
	  break;
	default:
	  if (pack.sig < 0)
	    sig_clear (-pack.sig);
	  else
	    {
	      int sigres = sig_handle (pack.sig, *pack.mask, pack.pid, pack.tls);
	      if (sigres <= 0)
		{
#ifdef DEBUGGING2
		  if (!sigres)
		    system_printf ("Failed to arm signal %d from pid %d", pack.sig, pack.pid);
#endif
		  sigqueue.add (pack.sig, pack.pid, pack.tls);// FIXME: Shouldn't add this in !sh condition
		}
	      if (pack.sig == SIGCHLD)
		proc_subproc (PROC_CLEARWAIT, 0);
	    }
	  break;
	}
      if (pack.wakeup)
	SetEvent (pack.wakeup);
    }

  sigproc_printf ("done");
  ExitThread (0);
}

/* Wait for subprocesses to terminate. Executes in a separate thread. */
static DWORD WINAPI
wait_subproc (VOID *)
{
  sigproc_printf ("starting");
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
	  if (!errloop++)
	    proc_subproc (PROC_NOTHING, 0);	// Just synchronize and continue
	  if (errloop < 10)
	    continue;

	  system_printf ("wait failed. nchildren %d, wait %d, %E",
			nchildren, proc_loop_wait);

	  for (int i = 0; i <= nchildren; i++)
	    if ((rc = WaitForSingleObject (events[i], 0)) == WAIT_OBJECT_0 ||
		rc == WAIT_TIMEOUT)
	      continue;
	    else if (i == 0)
		system_printf ("nchildren %d, event[%d] %p, %E", nchildren, i, events[i]);
	    else
	      {
		system_printf ("nchildren %d, event[%d] %p, pchildren[%d] %p, events[0] %p, %E",
			       nchildren, i, events[i], i - 1, (_pinfo *) pchildren[i - 1], events[0]);
		system_printf ("pid %d, dwProcessId %u, hProcess %p, progname '%s'",
			       pchildren[i - 1]->pid, pchildren[i - 1]->dwProcessId,
			       pchildren[i - 1]->hProcess, pchildren[i - 1]->progname);
	      }
	  break;
	}

      errloop = 0;
      rc -= WAIT_OBJECT_0;
      if (rc-- != 0)
	{
	  rc = proc_subproc (PROC_CHILDTERMINATED, rc);
	  if (!proc_loop_wait)		// Don't bother if wait_subproc is
	    break;			//  exiting

	  /* Send a SIGCHLD to myself.   We do this here, rather than in proc_subproc
	     to avoid the proc_subproc lock since the signal thread will eventually
	     be calling proc_subproc and could unnecessarily block. */
	  if (rc)
	    sig_send (myself_nowait, SIGCHLD);
	}
      sigproc_printf ("looping");
    }

  ForceCloseHandle (events[0]);
  events[0] = NULL;
  sigproc_printf ("done");
  ExitThread (0);
}
