/* sigproc.cc: inter/intra signal and sub process handler

   Copyright 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007 Red Hat, Inc.

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
#include "cygtls.h"
#include "sigproc.h"
#include "exceptions.h"

/*
 * Convenience defines
 */
#define WSSC		  60000	// Wait for signal completion
#define WPSP		  40000	// Wait for proc_subproc mutex

#define no_signals_available(x) (!hwait_sig || hwait_sig == INVALID_HANDLE_VALUE || ((x) && myself->exitcode & EXITCODE_SET) || &_my_tls == _sig_tls)

#define NPROCS	256

/*
 * Global variables
 */
struct sigaction *global_sigs;

const char *__sp_fn ;
int __sp_ln;

char NO_COPY myself_nowait_dummy[1] = {'0'};// Flag to sig_send that signal goes to
					//  current process but no wait is required
HANDLE NO_COPY signal_arrived;		// Event signaled when a signal has
					//  resulted in a user-specified
					//  function call

#define Static static NO_COPY

HANDLE NO_COPY sigCONT;			// Used to "STOP" a process

cygthread NO_COPY *hwait_sig;
Static HANDLE wait_sig_inited;		// Control synchronization of
					//  message queue startup
Static bool sigheld;			// True if holding signals

Static int nprocs;			// Number of deceased children
Static char cprocs[(NPROCS + 1) * sizeof (pinfo)];// All my children info
#define procs ((pinfo *) cprocs)	// All this just to avoid expensive
					// constructor operation  at DLL startup
Static waitq waitq_head;		// Start of queue for wait'ing threads

Static muto sync_proc_subproc;	// Control access to subproc stuff

_cygtls NO_COPY *_sig_tls;

Static HANDLE my_sendsig;
Static HANDLE my_readsig;

/* Function declarations */
static int __stdcall checkstate (waitq *) __attribute__ ((regparm (1)));
static __inline__ bool get_proc_lock (DWORD, DWORD);
static bool __stdcall remove_proc (int);
static bool __stdcall stopped_or_terminated (waitq *, _pinfo *);
static DWORD WINAPI wait_sig (VOID *arg);

/* wait_sig bookkeeping */

class pending_signals
{
  sigpacket sigs[NSIG + 1];
  sigpacket start;
  sigpacket *end;
  sigpacket *prev;
  sigpacket *curr;
public:
  void reset () {curr = &start; prev = &start;}
  void add (sigpacket&);
  void del ();
  sigpacket *next ();
  sigpacket *save () const {return curr;}
  void restore (sigpacket *saved) {curr = saved;}
  friend void __stdcall sig_dispatch_pending (bool);
  friend DWORD WINAPI wait_sig (VOID *arg);
};

Static pending_signals sigq;

/* Functions */
void __stdcall
sigalloc ()
{
  cygheap->sigs = global_sigs =
    (struct sigaction *) ccalloc_abort (HEAP_SIGS, NSIG, sizeof (struct sigaction));
  global_sigs[SIGSTOP].sa_flags = SA_RESTART | SA_NODEFER;
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
	{
	  global_sigs[i].sa_handler = SIG_DFL;
	  global_sigs[i].sa_flags &= ~ SA_SIGINFO;
	}
    }
}

void __stdcall
wait_for_sigthread (bool forked)
{
  char char_sa_buf[1024];
  PSECURITY_ATTRIBUTES sa_buf = sec_user_nih ((PSECURITY_ATTRIBUTES) char_sa_buf, cygheap->user.sid());
  if (!CreatePipe (&my_readsig, &my_sendsig, sa_buf, 0))
    api_fatal ("couldn't create signal pipe%s, %E", forked ? " for forked process" : "");
  ProtectHandle (my_readsig);
  myself->sendsig = my_sendsig;

  myself->process_state |= PID_ACTIVE;
  myself->process_state &= ~PID_INITIALIZING;

  sigproc_printf ("wait_sig_inited %p", wait_sig_inited);
  HANDLE hsig_inited = wait_sig_inited;
  WaitForSingleObject (hsig_inited, INFINITE);
  wait_sig_inited = NULL;
  ForceCloseHandle1 (hsig_inited, wait_sig_inited);
  SetEvent (sigCONT);
  sigproc_printf ("process/signal handling enabled, state %p", myself->process_state);
}

/* Get the sync_proc_subproc muto to control access to
 * children, proc arrays.
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
  if (sync_proc_subproc.acquire (WPSP))
    {
      lastwhat = what;
      return true;
    }
  if (!sync_proc_subproc)
    {
      sigproc_printf ("sync_proc_subproc is NULL (2)");
      return false;
    }
  system_printf ("Couldn't aquire sync_proc_subproc for(%d,%d), last %d, %E",
		  what, val, lastwhat);
  return true;
}

static bool __stdcall
proc_can_be_signalled (_pinfo *p)
{
  if (!(p->exitcode & EXITCODE_SET))
    {
      if (ISSTATE (p, PID_INITIALIZING) ||
	  (((p)->process_state & (PID_ACTIVE | PID_IN_USE)) ==
	   (PID_ACTIVE | PID_IN_USE)))
	return true;
    }

  set_errno (ESRCH);
  return false;
}

bool __stdcall
pid_exists (pid_t pid)
{
  return pinfo (pid)->exists ();
}

/* Return true if this is one of our children, false otherwise.  */
static inline bool __stdcall
mychild (int pid)
{
  for (int i = 0; i < nprocs; i++)
    if (procs[i]->pid == pid)
      return true;
  return false;
}

/* Handle all subprocess requests
 */
int __stdcall
proc_subproc (DWORD what, DWORD val)
{
  int rc = 1;
  int potential_match;
  _pinfo *child;
  int clearing;
  waitq *w;

#define wval	 ((waitq *) val)
#define vchild (*((pinfo *) val))

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
      /* Filled up process table? */
      if (nprocs >= NPROCS)
	{
	  sigproc_printf ("proc table overflow: hit %d processes, pid %d\n",
			  nprocs, vchild->pid);
	  rc = 0;
	  set_errno (EAGAIN);
	  break;
	}
      /* fall through intentionally */

    case PROC_DETACHED_CHILD:
      if (vchild != myself)
	{
	  vchild->ppid = what == PROC_DETACHED_CHILD ? 1 : myself->pid;
	  vchild->uid = myself->uid;
	  vchild->gid = myself->gid;
	  vchild->pgid = myself->pgid;
	  vchild->sid = myself->sid;
	  vchild->ctty = myself->ctty;
	  vchild->cygstarted = true;
	  vchild->process_state |= PID_INITIALIZING | (myself->process_state & PID_USETTY);
	}
      if (what == PROC_DETACHED_CHILD)
	break;
      procs[nprocs] = vchild;
      rc = procs[nprocs].wait ();
      if (rc)
	{
	  sigproc_printf ("added pid %d to proc table, slot %d", vchild->pid,
			  nprocs);
	  nprocs++;
	}
      break;

    /* Handle a wait4() operation.  Allocates an event for the calling
     * thread which is signaled when the appropriate pid exits or stops.
     * (usually called from the main thread)
     */
    case PROC_WAIT:
      wval->ev = NULL;		// Don't know event flag yet

      if (wval->pid == -1 || !wval->pid)
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
	  wval->ev = wval->thread_ev = CreateEvent (&sec_none_nih, TRUE, FALSE,
						    NULL);
	  ProtectHandle1 (wval->ev, wq_ev);
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
	sigproc_printf ("looking for processes to reap, nprocs %d", nprocs);
      clearing = val;

    scan_wait:
      /* Scan the linked list of wait()ing threads.  If a wait's parameters
	 match this pid, then activate it.  */
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
	for (int i = 0; i < nprocs; i += remove_proc (i))
	  continue;
  }

out:
  sync_proc_subproc.release ();	// Release the lock
out1:
  sigproc_printf ("returning %d", rc);
  return rc;
#undef wval
#undef vchild
}

// FIXME: This is inelegant
void
_cygtls::remove_wq (DWORD wait)
{
  if (exit_state < ES_FINAL && sync_proc_subproc
      && sync_proc_subproc.acquire (wait))
    {
      for (waitq *w = &waitq_head; w->next != NULL; w = w->next)
	if (w->next == &wq)
	  {
	    ForceCloseHandle1 (wq.thread_ev, wq_ev);
	    w->next = wq.next;
	    break;
	  }
      sync_proc_subproc.release ();
    }
}

/* Terminate the wait_subproc thread.
   Called on process exit.
   Also called by spawn_guts to disassociate any subprocesses from this
   process.  Subprocesses will then know to clean up after themselves and
   will not become procs.  */
void __stdcall
proc_terminate ()
{
  sigproc_printf ("nprocs %d", nprocs);
  if (nprocs)
    {
      sync_proc_subproc.acquire (WPSP);

      proc_subproc (PROC_CLEARWAIT, 1);

      /* Clean out proc processes from the pid list. */
      int i;
      for (i = 0; i < nprocs; i++)
	{
	  procs[i]->ppid = 1;
	  if (procs[i].wait_thread)
	    {
	      // CloseHandle (procs[i].rd_proc_pipe);
	      procs[i].wait_thread->terminate_thread ();
	    }
	  procs[i].release ();
	}
      nprocs = 0;
      sync_proc_subproc.release ();
    }
  sigproc_printf ("leaving");
}

/* Clear pending signal */
void __stdcall
sig_clear (int target_sig)
{
  if (&_my_tls != _sig_tls)
    sig_send (myself, -target_sig);
  else
    {
      sigpacket *q;
      sigpacket *save = sigq.save ();
      sigq.reset ();
      while ((q = sigq.next ()))
	if (q->si.si_signo == target_sig)
	  {
	    q->si.si_signo = __SIGDELETE;
	    break;
	  }
      sigq.restore (save);
    }
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
void __stdcall
sig_dispatch_pending (bool fast)
{
  if (exit_state || &_my_tls == _sig_tls)
    {
#ifdef DEBUGGING
      sigproc_printf ("exit_state %d, cur thread id %p, _sig_tls %p, sigq.start.next %p",
		      exit_state, GetCurrentThreadId (), _sig_tls, sigq.start.next);
#endif
      return;
    }

#ifdef DEBUGGING
  sigproc_printf ("flushing");
#endif
  sig_send (myself, fast ? __SIGFLUSHFAST : __SIGFLUSH);
}

void __stdcall
create_signal_arrived ()
{
  if (signal_arrived)
    return;
  /* local event signaled when main thread has been dispatched
     to a signal handler function. */
  signal_arrived = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
  ProtectHandle (signal_arrived);
}

/* Signal thread initialization.  Called from dll_crt0_1.

   This routine starts the signal handling thread.  The wait_sig_inited
   event is used to signal that the thread is ready to handle signals.
   We don't wait for this during initialization but instead detect it
   in sig_send to gain a little concurrency.  */
void __stdcall
sigproc_init ()
{
  wait_sig_inited = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
  ProtectHandle (wait_sig_inited);

  /* sync_proc_subproc is used by proc_subproc.  It serialises
     access to the children and proc arrays.  */
  sync_proc_subproc.init ("sync_proc_subproc");

  hwait_sig = new cygthread (wait_sig, 0, cygself, "sig");
  hwait_sig->zap_h ();
}

/* Called on process termination to terminate signal and process threads.
 */
void __stdcall
sigproc_terminate (exit_states es)
{
  exit_states prior_exit_state = exit_state;
  exit_state = es;
  if (prior_exit_state >= ES_FINAL)
    sigproc_printf ("already performed");
  else
    {
      sigproc_printf ("entering");
      sig_send (myself_nowait, __SIGEXIT);
      proc_terminate ();		// clean up process stuff
    }
}

int __stdcall
sig_send (_pinfo *p, int sig)
{
  if (sig == __SIGHOLD)
    sigheld = true;
  else if (!sigheld)
    /* nothing */;
  else if (sig == __SIGFLUSH || sig == __SIGFLUSHFAST)
    return 0;
  else if (sig == __SIGNOHOLD || sig == __SIGEXIT)
    {
      SetEvent (sigCONT);
      sigheld = false;
    }
  else if (&_my_tls == _main_tls)
    {
#ifdef DEBUGGING
      system_printf ("signal %d sent to %p while signals are on hold", sig, p);
#endif
      return -1;
    }
  siginfo_t si = {0};
  si.si_signo = sig;
  si.si_code = SI_KERNEL;
  si.si_pid = si.si_uid = si.si_errno = 0;
  return sig_send (p, si);
}

/* Send a signal to another process by raising its signal semaphore.
   If pinfo *p == NULL, send to the current process.
   If sending to this process, wait for notification that a signal has
   completed before returning.  */
int __stdcall
sig_send (_pinfo *p, siginfo_t& si, _cygtls *tls)
{
  int rc = 1;
  bool its_me;
  HANDLE sendsig;
  sigpacket pack;
  bool communing = si.si_signo == __SIGCOMMUNE;

  pack.wakeup = NULL;
  bool wait_for_completion;
  if (!(its_me = (p == NULL || p == myself || p == myself_nowait)))
    {
      /* It is possible that the process is not yet ready to receive messages
       * or that it has exited.  Detect this.
       */
      if (!proc_can_be_signalled (p))	/* Is the process accepting messages? */
	{
	  sigproc_printf ("invalid pid %d(%x), signal %d",
			  p->pid, p->process_state, si.si_signo);
	  goto out;
	}
      wait_for_completion = false;
    }
  else
    {
      if (no_signals_available (si.si_signo != __SIGEXIT))
	{
	  sigproc_printf ("my_sendsig %p, myself->sendsig %p, exit_state %d",
			  my_sendsig, myself->sendsig, exit_state);
	  set_errno (EAGAIN);
	  goto out;		// Either exiting or not yet initializing
	}
      if (wait_sig_inited)
	wait_for_sigthread ();
      wait_for_completion = p != myself_nowait && _my_tls.isinitialized () && !exit_state;
      p = myself;
    }


  if (its_me)
    sendsig = my_sendsig;
  else
    {
      HANDLE dupsig;
      DWORD dwProcessId;
      for (int i = 0; !p->sendsig && i < 10000; i++)
	low_priority_sleep (0);
      if (p->sendsig)
	{
	  dupsig = p->sendsig;
	  dwProcessId = p->dwProcessId;
	}
      else
	{
	  dupsig = p->exec_sendsig;
	  dwProcessId = p->exec_dwProcessId;
	}
      if (!dupsig)
	{
	  set_errno (EAGAIN);
	  sigproc_printf ("sendsig handle never materialized");
	  goto out;
	}
      HANDLE hp = OpenProcess (PROCESS_DUP_HANDLE, false, dwProcessId);
      if (!hp)
	{
	  __seterrno ();
	  sigproc_printf ("OpenProcess failed, %E");
	  goto out;
	}
      VerifyHandle (hp);
      if (!DuplicateHandle (hp, dupsig, hMainProc, &sendsig, false, 0,
			    DUPLICATE_SAME_ACCESS) || !sendsig)
	{
	  __seterrno ();
	  sigproc_printf ("DuplicateHandle failed, %E");
	  CloseHandle (hp);
	  goto out;
	}
      VerifyHandle (sendsig);
      if (!communing)
	CloseHandle (hp);
      else
	{
	  si._si_commune._si_process_handle = hp;

	  HANDLE& tome = si._si_commune._si_write_handle;
	  HANDLE& fromthem = si._si_commune._si_read_handle;
	  if (!CreatePipe (&fromthem, &tome, &sec_all_nih, 0))
	    {
	      sigproc_printf ("CreatePipe for __SIGCOMMUNE failed, %E");
	      __seterrno ();
	      goto out;
	    }
	  if (!DuplicateHandle (hMainProc, tome, hp, &tome, false, 0,
				DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
	    {
	      sigproc_printf ("DuplicateHandle for __SIGCOMMUNE failed, %E");
	      __seterrno ();
	      goto out;
	    }
	}
    }

  sigproc_printf ("sendsig %p, pid %d, signal %d, its_me %d", sendsig, p->pid, si.si_signo, its_me);

  sigset_t pending;
  if (!its_me)
    pack.mask = NULL;
  else if (si.si_signo == __SIGPENDING)
    pack.mask = &pending;
  else if (si.si_signo == __SIGFLUSH || si.si_signo > 0)
    pack.mask = tls ? &tls->sigmask : &_main_tls->sigmask;
  else
    pack.mask = NULL;

  pack.si = si;
  if (!pack.si.si_pid)
    pack.si.si_pid = myself->pid;
  if (!pack.si.si_uid)
    pack.si.si_uid = myself->uid;
  pack.pid = myself->pid;
  pack.tls = tls;
  if (wait_for_completion)
    {
      pack.wakeup = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
      sigproc_printf ("wakeup %p", pack.wakeup);
      ProtectHandle (pack.wakeup);
    }

  char *leader;
  size_t packsize;
  if (!communing || !(si._si_commune._si_code & PICOM_EXTRASTR))
    {
      leader = (char *) &pack;
      packsize = sizeof (pack);
    }
  else
    {
      size_t n = strlen (si._si_commune._si_str);
      char *p = leader = (char *) alloca (sizeof (pack) + sizeof (n) + n);
      memcpy (p, &pack, sizeof (pack)); p += sizeof (pack);
      memcpy (p, &n, sizeof (n)); p += sizeof (n);
      memcpy (p, si._si_commune._si_str, n); p += n;
      packsize = p - leader;
    }

  DWORD nb;
  if (!WriteFile (sendsig, leader, packsize, &nb, NULL) || nb != packsize)
    {
      /* Couldn't send to the pipe.  This probably means that the
	 process is exiting.  */
      if (!its_me)
	{
	  __seterrno ();
	  sigproc_printf ("WriteFile for pipe %p failed, %E", sendsig);
	  ForceCloseHandle (sendsig);
	}
      else
	{
	  if (no_signals_available (true))
	    sigproc_printf ("I'm going away now");
	  else if (!p->exec_sendsig)
	    system_printf ("error sending signal %d to pid %d, pipe handle %p, %E",
			   si.si_signo, p->pid, sendsig);
	  set_errno (EACCES);
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
      ForceCloseHandle (pack.wakeup);
    }
  else
    {
      rc = WAIT_OBJECT_0;
      sigproc_printf ("Not waiting for sigcomplete.  its_me %d signal %d",
		      its_me, si.si_signo);
      if (!its_me)
	ForceCloseHandle (sendsig);
    }

  pack.wakeup = NULL;
  if (rc == WAIT_OBJECT_0)
    rc = 0;		// Successful exit
  else
    {
      if (!no_signals_available (true))
	system_printf ("wait for sig_complete event failed, signal %d, rc %d, %E",
		       si.si_signo, rc);
      set_errno (ENOSYS);
      rc = -1;
    }

  if (wait_for_completion && si.si_signo != __SIGFLUSHFAST)
    _my_tls.call_signal_handler ();
  goto out;

out:
  if (communing && rc)
    {
      if (si._si_commune._si_process_handle)
	CloseHandle (si._si_commune._si_process_handle);
      if (si._si_commune._si_read_handle)
	CloseHandle (si._si_commune._si_read_handle);
    }
  if (pack.wakeup)
    ForceCloseHandle (pack.wakeup);
  if (si.si_signo != __SIGPENDING)
    /* nothing */;
  else if (!rc)
    rc = (int) pending;
  else
    rc = SIG_BAD_MASK;
  sigproc_printf ("returning %p from sending signal %d", rc, si.si_signo);
  return rc;
}

int child_info::retry_count = 10;
/* Initialize some of the memory block passed to child processes
   by fork/spawn/exec. */

child_info::child_info (unsigned in_cb, child_info_types chtype, bool need_subproc_ready)
{
  memset (this, 0, in_cb);
  cb = in_cb;

  /* It appears that when running under WOW64 on Vista 64, the first DWORD
     value in the datastructure lpReserved2 is pointing to (msv_count in
     Cygwin), has to reflect the size of that datastructure as used in the
     Microsoft C runtime (a count value, counting the number of elements in
     two subsequent arrays, BYTE[count and HANDLE[count]), even though the C
     runtime isn't used.  Otherwise, if msv_count is 0 or too small, the
     datastructure gets overwritten.

     This seems to be a bug in Vista's WOW64, which apparently copies the
     lpReserved2 datastructure not using the cbReserved2 size information,
     but using the information given in the first DWORD within lpReserved2
     instead.  32 bit Windows and former WOW64 don't care if msv_count is 0
     or a sensible non-0 count value.  However, it's not clear if a non-0
     count doesn't result in trying to evaluate the content, so we do this
     really only for Vista 64 for now.

     Note: It turns out that a non-zero value *does* harm operation on
     XP 64 and 2K3 64 (Crash in CreateProcess call).

     The value is sizeof (child_info_*) / 5 which results in a count which
     covers the full datastructure, plus not more than 4 extra bytes.  This
     is ok as long as the child_info structure is cosily stored within a bigger
     datastructure. */
  msv_count = wincap.needs_count_in_si_lpres2 () ? in_cb / 5 : 0;

  intro = PROC_MAGIC_GENERIC;
  magic = CHILD_INFO_MAGIC;
  type = chtype;
  fhandler_union_cb = sizeof (fhandler_union);
  user_h = cygwin_user_h;
  if (strace.attached ())
    flag |= _CI_STRACED;
  if (need_subproc_ready)
    {
      subproc_ready = CreateEvent (&sec_all, FALSE, FALSE, NULL);
      flag |= _CI_ISCYGWIN;
    }
  sigproc_printf ("subproc_ready %p", subproc_ready);
  cygheap = ::cygheap;
  cygheap_max = ::cygheap_max;
  retry = child_info::retry_count;
  /* Create an inheritable handle to pass to the child process.  This will
     allow the child to duplicate handles from the parent to itself. */
  parent = NULL;
  if (!DuplicateHandle (hMainProc, hMainProc, hMainProc, &parent, 0, TRUE,
			DUPLICATE_SAME_ACCESS))
    system_printf ("couldn't create handle to myself for child, %E");
}

child_info::~child_info ()
{
  if (subproc_ready)
    CloseHandle (subproc_ready);
  if (parent)
    CloseHandle (parent);
}

child_info_fork::child_info_fork () :
  child_info (sizeof *this, _PROC_FORK, true)
{
}

child_info_spawn::child_info_spawn (child_info_types chtype, bool need_subproc_ready) :
  child_info (sizeof *this, chtype, need_subproc_ready)
{
}

void
child_info::ready (bool execed)
{
  if (!subproc_ready)
    {
      sigproc_printf ("subproc_ready not set");
      return;
    }

  if (dynamically_loaded)
    sigproc_printf ("not really ready");
  else if (!SetEvent (subproc_ready))
    api_fatal ("SetEvent failed");
  else
    sigproc_printf ("signalled %p that I was ready", subproc_ready);

  if (execed)
    {
      CloseHandle (subproc_ready);
      subproc_ready = NULL;
    }
}

bool
child_info::sync (pid_t pid, HANDLE& hProcess, DWORD howlong)
{
  bool res;
  HANDLE w4[2];
  unsigned n = 0;
  unsigned nsubproc_ready;

  if (!subproc_ready)
    nsubproc_ready = WAIT_OBJECT_0 + 3;
  else
    {
      w4[n++] = subproc_ready;
      nsubproc_ready = 0;
    }
  w4[n++] = hProcess;

  sigproc_printf ("n %d, waiting for subproc_ready(%p) and child process(%p)", n, w4[0], w4[1]);
  DWORD x = WaitForMultipleObjects (n, w4, FALSE, howlong);
  x -= WAIT_OBJECT_0;
  if (x >= n)
    {
      system_printf ("wait failed, pid %u, %E", pid);
      res = false;
    }
  else
    {
      if (x != nsubproc_ready)
	{
	  res = false;
	  GetExitCodeProcess (hProcess, &exit_code);
	}
      else
	{
	  res = true;
	  exit_code = STILL_ACTIVE;
	  if (type == _PROC_EXEC && myself->wr_proc_pipe)
	    {
	      ForceCloseHandle1 (hProcess, childhProc);
	      hProcess = NULL;
	    }
	}
      sigproc_printf ("pid %u, WFMO returned %d, res %d", pid, x, res);
    }
  return res;
}

DWORD
child_info::proc_retry (HANDLE h)
{
  if (!exit_code)
    return EXITCODE_OK;
  switch (exit_code)
    {
    case STILL_ACTIVE:	/* shouldn't happen */
      sigproc_printf ("STILL_ACTIVE?  How'd we get here?");
      break;
    case STATUS_CONTROL_C_EXIT:
      if (saw_ctrl_c ())
	return EXITCODE_OK;
      /* fall through intentionally */
    case STATUS_DLL_INIT_FAILED:
    case STATUS_DLL_INIT_FAILED_LOGOFF:
    case EXITCODE_RETRY:
      if (retry-- > 0)
	exit_code = 0;
      break;
    /* Count down non-recognized exit codes more quickly since they aren't
       due to known conditions.  */
    default:
      if (!iscygwin () && (exit_code & 0xffff0000) != 0xc0000000)
	break;
      if ((retry -= 2) < 0)
	retry = 0;
      else
	exit_code = 0;
    }
  if (!exit_code)
    ForceCloseHandle1 (h, childhProc);
  return exit_code;
}

bool
child_info_fork::handle_failure (DWORD err)
{
  if (retry > 0)
    ExitProcess (EXITCODE_RETRY);
  return 0;
}

/* Check the state of all of our children to see if any are stopped or
 * terminated.
 */
static int __stdcall
checkstate (waitq *parent_w)
{
  int potential_match = 0;

  sigproc_printf ("nprocs %d", nprocs);

  /* Check already dead processes first to see if they match the criteria
   * given in w->next.  */
  int res;
  for (int i = 0; i < nprocs; i++)
    if ((res = stopped_or_terminated (parent_w, procs[i])))
      {
	remove_proc (i);
	potential_match = 1;
	goto out;
      }

  sigproc_printf ("no matching terminated children found");
  potential_match = -!!nprocs;

out:
  sigproc_printf ("returning %d", potential_match);
  return potential_match;
}

/* Remove a proc from procs by swapping it with the last child in the list.
   Also releases shared memory of exited processes.  */
static bool __stdcall
remove_proc (int ci)
{
  if (procs[ci]->exists ())
    return true;

  sigproc_printf ("removing procs[%d], pid %d, nprocs %d", ci, procs[ci]->pid,
		  nprocs);
  if (procs[ci] != myself)
    {
      procs[ci].release ();
      if (procs[ci].hProcess)
	ForceCloseHandle1 (procs[ci].hProcess, childhProc);
    }
  if (ci < --nprocs)
    {
      /* Wait for proc_waiter thread to make a copy of this element before
	 moving it or it may become confused.  The chances are very high that
	 the proc_waiter thread has already done this by the time we
	 get here.  */
      while (!procs[nprocs].waiter_ready)
	low_priority_sleep (0);
      procs[ci] = procs[nprocs];
    }
  return 0;
}

/* Check status of child process vs. waitq member.

   parent_w is the pointer to the parent of the waitq member in question.
   child is the subprocess being considered.

   Returns non-zero if waiting thread released.  */
static bool __stdcall
stopped_or_terminated (waitq *parent_w, _pinfo *child)
{
  int might_match;
  waitq *w = parent_w->next;

  sigproc_printf ("considering pid %d", child->pid);
  if (w->pid == -1)
    might_match = 1;
  else if (w->pid == 0)
    might_match = child->pgid == myself->pgid;
  else if (w->pid < 0)
    might_match = child->pgid == -w->pid;
  else
    might_match = (w->pid == child->pid);

  if (!might_match)
    return 0;

  int terminated;

  if (!((terminated = (child->process_state == PID_EXITED)) ||
      ((w->options & WUNTRACED) && child->stopsig)))
    return 0;

  parent_w->next = w->next;	/* successful wait.  remove from wait queue */
  w->pid = child->pid;

  if (!terminated)
    {
      sigproc_printf ("stopped child");
      w->status = (child->stopsig << 8) | 0x7f;
      child->stopsig = 0;
    }
  else
    {
      w->status = (__uint16_t) child->exitcode;

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
  return true;
}

static void
talktome (siginfo_t *si)
{
  unsigned size = sizeof (*si);
  sigproc_printf ("pid %d wants some information", si->si_pid);
  if (si->_si_commune._si_code & PICOM_EXTRASTR)
    {
      size_t n;
      DWORD nb;
      if (!ReadFile (my_readsig, &n, sizeof (n), &nb, NULL) || nb != sizeof (n))
	return;
      siginfo_t *newsi = (siginfo_t *) alloca (size += n + 1);
      *newsi = *si;
      newsi->_si_commune._si_str = (char *) (newsi + 1);
      if (!ReadFile (my_readsig, newsi->_si_commune._si_str, n, &nb, NULL) || nb != n)
	return;
      newsi->_si_commune._si_str[n] = '\0';
      si = newsi;
    }

  pinfo pi (si->si_pid);
  if (pi)
    new cygthread (commune_process, size, si, "commune_process");
}

void
pending_signals::add (sigpacket& pack)
{
  sigpacket *se;
  if (sigs[pack.si.si_signo].si.si_signo)
    return;
  se = sigs + pack.si.si_signo;
  *se = pack;
  se->mask = &pack.tls->sigmask;
  se->next = NULL;
  if (end)
    end->next = se;
  end = se;
  if (!start.next)
    start.next = se;
}

void
pending_signals::del ()
{
  sigpacket *next = curr->next;
  prev->next = next;
  curr->si.si_signo = 0;
#ifdef DEBUGGING
  curr->next = NULL;
#endif
  if (end == curr)
    end = prev;
  curr = next;
}

sigpacket *
pending_signals::next ()
{
  sigpacket *res;
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
wait_sig (VOID *)
{
  /* Initialization */
  SetThreadPriority (GetCurrentThread (), WAIT_SIG_PRIORITY);

  sigCONT = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);

  SetEvent (wait_sig_inited);

  _sig_tls = &_my_tls;
  _sig_tls->init_threadlist_exceptions ();
  sigproc_printf ("entering ReadFile loop, my_readsig %p, my_sendsig %p",
		  my_readsig, my_sendsig);

  sigpacket pack;
  pack.si.si_signo = __SIGHOLD;
  for (;;)
    {
      if (pack.si.si_signo == __SIGHOLD)
	WaitForSingleObject (sigCONT, INFINITE);
      DWORD nb;
      pack.tls = NULL;
      if (!ReadFile (my_readsig, &pack, sizeof (pack), &nb, NULL))
	break;

      if (nb != sizeof (pack))
	{
	  system_printf ("short read from signal pipe: %d != %d", nb,
			 sizeof (pack));
	  continue;
	}

      if (!pack.si.si_signo)
	{
#ifdef DEBUGGING
	  system_printf ("zero signal?");
#endif
	  continue;
	}

      sigset_t dummy_mask;
      if (!pack.mask)
	{
	  dummy_mask = _main_tls->sigmask;
	  pack.mask = &dummy_mask;
	}

      sigpacket *q;
      bool clearwait = false;
      switch (pack.si.si_signo)
	{
	case __SIGCOMMUNE:
	  talktome (&pack.si);
	  break;
	case __SIGSTRACE:
	  strace.hello ();
	  break;
	case __SIGPENDING:
	  *pack.mask = 0;
	  unsigned bit;
	  sigq.reset ();
	  while ((q = sigq.next ()))
	    if (pack.tls->sigmask & (bit = SIGTOMASK (q->si.si_signo)))
	      *pack.mask |= bit;
	  break;
	case __SIGHOLD:
	  goto loop;
	  break;
	case __SIGNOHOLD:
	case __SIGFLUSH:
	case __SIGFLUSHFAST:
	  sigq.reset ();
	  while ((q = sigq.next ()))
	    {
	      int sig = q->si.si_signo;
	      if (sig == __SIGDELETE || q->process () > 0)
		sigq.del ();
	      if (sig == __SIGNOHOLD && q->si.si_signo == SIGCHLD)
		clearwait = true;
	    }
	  break;
	case __SIGEXIT:
	  hwait_sig = (cygthread *) INVALID_HANDLE_VALUE;
	  sigproc_printf ("saw __SIGEXIT");
	  break;	/* handle below */
	default:
	  if (pack.si.si_signo < 0)
	    sig_clear (-pack.si.si_signo);
	  else
	    {
	      int sig = pack.si.si_signo;
	      // FIXME: REALLY not right when taking threads into consideration.
	      // We need a per-thread queue since each thread can have its own
	      // list of blocked signals.  CGF 2005-08-24
	      if (sigq.sigs[sig].si.si_signo && sigq.sigs[sig].tls == pack.tls)
		sigproc_printf ("sig %d already queued", pack.si.si_signo);
	      else
		{
		  int sigres = pack.process ();
		  if (sigres <= 0)
		    {
#ifdef DEBUGGING2
		      if (!sigres)
			system_printf ("Failed to arm signal %d from pid %d", pack.sig, pack.pid);
#endif
		      sigq.add (pack);	// FIXME: Shouldn't add this in !sh condition
		    }
		}
	      if (sig == SIGCHLD)
		clearwait = true;
	    }
	  break;
	}
      if (clearwait)
	proc_subproc (PROC_CLEARWAIT, 0);
    loop:
      if (pack.wakeup)
	{
	  sigproc_printf ("signalling pack.wakeup %p", pack.wakeup);
	  SetEvent (pack.wakeup);
	}
      if (pack.si.si_signo == __SIGEXIT)
	break;
    }

  ForceCloseHandle (my_readsig);
  sigproc_printf ("signal thread exiting");
  ExitThread (0);
}
