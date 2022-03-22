/* sigproc.cc: inter/intra signal and sub process handler

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <stdlib.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "sigproc.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "child_info_magic.h"
#include "shared_info.h"
#include "cygtls.h"
#include "ntdll.h"
#include "exception.h"

/*
 * Convenience defines
 */
#define WSSC		  60000	// Wait for signal completion
#define WPSP		  40000	// Wait for proc_subproc mutex

/*
 * Global variables
 */
struct sigaction *global_sigs;

const char *__sp_fn ;
int __sp_ln;

bool no_thread_exit_protect::flag;

/* Flag to sig_send that signal goes to current process but no wait is
   required. */
char NO_COPY myself_nowait_dummy[1] = {'0'};

#define Static static NO_COPY

/* All my children info.  Avoid expensive constructor ops at DLL
   startup.

   This class can allocate memory.  But there's no need to free it
   because only one instance of the class is created per process. */
class child_procs {
#ifdef __i386__
    static const int _NPROCS = 256;
    static const int _NPROCS_2 = 1023;
#else
    static const int _NPROCS = 1024;
    static const int _NPROCS_2 = 4095;
#endif
    int _count;
    uint8_t _procs[_NPROCS * sizeof (pinfo)] __attribute__ ((__aligned__));
    pinfo *_procs_2;
  public:
    int count () const { return _count; }
    int add_one () { return ++_count; }
    int del_one () { return --_count; }
    int reset () { return _count = 0; }
    pinfo &operator[] (int idx)
    {
      if (idx >= _NPROCS)
	{
	  if (!_procs_2)
	    {
	      /* Use HeapAlloc to avoid propagating this memory area
		 to the child processes. */
	      _procs_2 = (pinfo *) HeapAlloc (GetProcessHeap (),
					      HEAP_GENERATE_EXCEPTIONS
					      | HEAP_ZERO_MEMORY,
					      (_NPROCS_2 + 1) * sizeof (pinfo));
	    }
	  return _procs_2[idx - _NPROCS];
	}
      return ((pinfo *) _procs)[idx];
    }
    int max_child_procs () const { return _NPROCS + _NPROCS_2; }
};
Static child_procs chld_procs;

/* Start of queue for waiting threads. */
Static waitq waitq_head;

/* Controls access to subproc stuff. */
Static muto sync_proc_subproc;

_cygtls NO_COPY *_sig_tls;

Static HANDLE my_sendsig;
Static HANDLE my_readsig;

/* Used in select if a signalfd is part of the read descriptor set */
HANDLE NO_COPY my_pendingsigs_evt;

/* Function declarations */
static int __reg1 checkstate (waitq *);
static __inline__ bool get_proc_lock (DWORD, DWORD);
static int remove_proc (int);
static bool stopped_or_terminated (waitq *, _pinfo *);
static void WINAPI wait_sig (VOID *arg);

/* wait_sig bookkeeping */

class pending_signals
{
  sigpacket sigs[_NSIG + 1];
  sigpacket start;
  bool retry;

public:
  void add (sigpacket&);
  bool pending () {retry = true; return !!start.next;}
  void clear (int sig) {sigs[sig].si.si_signo = 0;}
  void clear (_cygtls *tls);
  friend void __reg1 sig_dispatch_pending (bool);
  friend void WINAPI wait_sig (VOID *arg);
};

Static pending_signals sigq;

/* Functions */
void
sigalloc ()
{
  cygheap->sigs = global_sigs =
    (struct sigaction *) ccalloc_abort (HEAP_SIGS, _NSIG, sizeof (struct sigaction));
  global_sigs[SIGSTOP].sa_flags = SA_RESTART | SA_NODEFER;
}

void
signal_fixup_after_exec ()
{
  global_sigs = cygheap->sigs;
  /* Set up child's signal handlers */
  for (int i = 0; i < _NSIG; i++)
    {
      global_sigs[i].sa_mask = 0;
      if (global_sigs[i].sa_handler != SIG_IGN)
	{
	  global_sigs[i].sa_handler = SIG_DFL;
	  global_sigs[i].sa_flags &= ~ SA_SIGINFO;
	}
    }
}

/* Get the sync_proc_subproc muto to control access to
 * children, proc arrays.
 * Attempt to handle case where process is exiting as we try to grab
 * the mutex.
 */
static bool
get_proc_lock (DWORD what, DWORD val)
{
  if (!cygwin_finished_initializing)
    return true;
  Static int lastwhat = -1;
  if (!sync_proc_subproc)
    {
      sigproc_printf ("sync_proc_subproc is NULL");
      return false;
    }
  if (sync_proc_subproc.acquire (WPSP))
    {
      lastwhat = what;
      return true;
    }
  system_printf ("Couldn't acquire %s for(%d,%d), last %d, %E",
		 sync_proc_subproc.name, what, val, lastwhat);
  return false;
}

static bool
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

bool __reg1
pid_exists (pid_t pid)
{
  pinfo p (pid);
  return p && p->exists ();
}

/* Return true if this is one of our children, false otherwise.  */
static inline bool
mychild (int pid)
{
  for (int i = 0; i < chld_procs.count (); i++)
    if (chld_procs[i]->pid == pid)
      return true;
  return false;
}

/* Handle all subprocess requests
 */
int __reg2
proc_subproc (DWORD what, uintptr_t val)
{
  int slot;
  int rc = 1;
  int potential_match;
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
    case PROC_ADD_CHILD:
      /* Filled up process table? */
      if (chld_procs.count () >= chld_procs.max_child_procs ())
	{
	  sigproc_printf ("proc table overflow: hit %d processes, pid %d\n",
			  chld_procs.count (), vchild->pid);
	  rc = 0;
	  set_errno (EAGAIN);
	  break;
	}

      if (vchild != myself)
	{
	  vchild->uid = myself->uid;
	  vchild->gid = myself->gid;
	  vchild->pgid = myself->pgid;
	  vchild->sid = myself->sid;
	  vchild->ctty = myself->ctty;
	  vchild->cygstarted = true;
	  vchild->process_state |= PID_INITIALIZING;
	  vchild->ppid = myself->pid;	/* always set last */
	}
      break;

    case PROC_ATTACH_CHILD:
      slot = chld_procs.count ();
      chld_procs[slot] = vchild;
      rc = chld_procs[slot].wait ();
      if (rc)
	{
	  sigproc_printf ("added pid %d to proc table, slot %d", vchild->pid,
			  slot);
	  chld_procs.add_one ();
	}
      break;

    /* Handle a wait4() operation.  Allocates an event for the calling
     * thread which is signaled when the appropriate pid exits or stops.
     * (usually called from the main thread)
     */
    case PROC_WAIT:
      wval->ev = NULL;		// Don't know event flag yet

      if (wval->pid != -1 && wval->pid && !mychild (wval->pid))
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
      clearing = false;
      goto scan_wait;

    case PROC_EXEC_CLEANUP:
      /* Cleanup backwards to eliminate redundant copying of chld_procs
	 array members inside remove_proc. */
      while (chld_procs.count ())
	remove_proc (chld_procs.count () - 1);
      for (w = &waitq_head; w->next != NULL; w = w->next)
	CloseHandle (w->next->ev);
      break;

    /* Clear all waiting threads.  Called from exceptions.cc prior to
       the main thread's dispatch to a signal handler function.
       (called from wait_sig thread) */
    case PROC_CLEARWAIT:
      /* Clear all "wait"ing threads. */
      if (val)
	sigproc_printf ("clear waiting threads");
      else
	sigproc_printf ("looking for processes to reap, count %d",
			chld_procs.count ());
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
	for (int i = 0; i < chld_procs.count (); i += remove_proc (i))
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
  if (wq.thread_ev)
    {
      if (exit_state < ES_FINAL && waitq_head.next && sync_proc_subproc
	  && sync_proc_subproc.acquire (wait))
	{
	  ForceCloseHandle1 (wq.thread_ev, wq_ev);
	  wq.thread_ev = NULL;
	  for (waitq *w = &waitq_head; w->next != NULL; w = w->next)
	    if (w->next == &wq)
	      {
		w->next = wq.next;
		break;
	      }
	  sync_proc_subproc.release ();
	}
    }

}

/* Terminate the wait_subproc thread.
   Called on process exit.
   Also called by spawn_guts to disassociate any subprocesses from this
   process.  Subprocesses will then know to clean up after themselves and
   will not become chld_procs.  */
void
proc_terminate ()
{
  sigproc_printf ("child_procs count %d", chld_procs.count ());
  if (chld_procs.count ())
    {
      sync_proc_subproc.acquire (WPSP);

      proc_subproc (PROC_CLEARWAIT, 1);

      /* Clean out proc processes from the pid list. */
      for (int i = 0; i < chld_procs.count (); i++)
	{
	  /* If we've execed then the execed process will handle setting ppid
	     to 1 iff it is a Cygwin process.  */
	  if (!have_execed || !have_execed_cygwin)
	    chld_procs[i]->ppid = 1;
	  if (chld_procs[i].wait_thread)
	    chld_procs[i].wait_thread->terminate_thread ();
	  /* Release memory associated with this process unless it is 'myself'.
	     'myself' is only in the chld_procs table when we've execed.  We
	     reach here when the next process has finished initializing but we
	     still can't free the memory used by 'myself' since it is used
	     later on during cygwin tear down.  */
	  if (chld_procs[i] != myself)
	    chld_procs[i].release ();
	}
      chld_procs.reset ();
      sync_proc_subproc.release ();
    }
  sigproc_printf ("leaving");
}

/* Clear pending signal */
void __reg1
sig_clear (int sig)
{
  sigq.clear (sig);
}

/* Clear pending signals of specific thread.  Called under TLS lock from
   _cygtls::remove_pending_sigs. */
void
pending_signals::clear (_cygtls *tls)
{
  sigpacket *q = &start, *qnext;

  while ((qnext = q->next))
    if (qnext->sigtls == tls)
      {
	qnext->si.si_signo = 0;
	q->next = qnext->next;
      }
    else
      q = qnext;
}

/* Clear pending signals of specific thread.  Called from _cygtls::remove */
void
_cygtls::remove_pending_sigs ()
{
  sigq.clear (this);
}

extern "C" int
sigpending (sigset_t *mask)
{
  sigset_t outset = sig_send (myself, __SIGPENDING, &_my_tls);
  if (outset == SIG_BAD_MASK)
    return -1;
  *mask = outset;
  return 0;
}

/* Force the wait_sig thread to wake up and scan for pending signals */
void __reg1
sig_dispatch_pending (bool fast)
{
  /* Non-atomically test for any signals pending and wake up wait_sig if any are
     found.  It's ok if there's a race here since the next call to this function
     should catch it.  */
  if (sigq.pending () && &_my_tls != _sig_tls)
    sig_send (myself, fast ? __SIGFLUSHFAST : __SIGFLUSH);
}

/* Signal thread initialization.  Called from dll_crt0_1.
   This routine starts the signal handling thread.  */
void
sigproc_init ()
{
  char char_sa_buf[1024];
  PSECURITY_ATTRIBUTES sa = sec_user_nih ((PSECURITY_ATTRIBUTES) char_sa_buf, cygheap->user.sid());
  DWORD err = fhandler_pipe::create (sa, &my_readsig, &my_sendsig,
				     _NSIG * sizeof (sigpacket), "sigwait",
				     PIPE_ADD_PID);
  if (err)
    {
      SetLastError (err);
      api_fatal ("couldn't create signal pipe, %E");
    }
  ProtectHandle (my_readsig);
  myself->sendsig = my_sendsig;
  my_pendingsigs_evt = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (!my_pendingsigs_evt)
    api_fatal ("couldn't create pending signal event, %E");

  /* sync_proc_subproc is used by proc_subproc.  It serializes
     access to the children and proc arrays.  */
  sync_proc_subproc.init ("sync_proc_subproc");
  new cygthread (wait_sig, cygself, "sig");
}

/* Exit the current thread very carefully.
   See cgf-000017 in DevNotes for more details on why this is
   necessary.  */
void
exit_thread (DWORD res)
{
# undef ExitThread
  if (no_thread_exit_protect ())
    ExitThread (res);
  sigfillset (&_my_tls.sigmask);	/* No signals wanted */

  /* CV 2014-11-21: Disable the code sending a signal.  The problem with
     this code is that it allows deadlocks under signal-rich multithreading
     conditions.
     The original problem reported in 2012 couldn't be reproduced anymore,
     even disabling this code.  Tested on XP 32, Vista 32, W7 32, WOW64, 64,
     W8.1 WOW64, 64. */
#if 0
  lock_process for_now;			/* May block indefinitely when exiting. */
  HANDLE h;
  if (!DuplicateHandle (GetCurrentProcess (), GetCurrentThread (),
                        GetCurrentProcess (), &h,
                        0, FALSE, DUPLICATE_SAME_ACCESS))
    {
#ifdef DEBUGGING
      system_printf ("couldn't duplicate the current thread, %E");
#endif
      for_now.release ();
      ExitThread (res);
    }
  ProtectHandle1 (h, exit_thread);
  /* Tell wait_sig to wait for this thread to exit.  It can then release
     the lock below and close the above-opened handle. */
  siginfo_t si = {__SIGTHREADEXIT, SI_KERNEL};
  si.si_cyg = h;
  sig_send (myself_nowait, si, &_my_tls);
#endif
  ExitThread (res);
}

sigset_t __reg3
sig_send (_pinfo *p, int sig, _cygtls *tls)
{
  siginfo_t si = {};
  si.si_signo = sig;
  si.si_code = SI_KERNEL;
  return sig_send (p, si, tls);
}

/* Send a signal to another process by raising its signal semaphore.
   If pinfo *p == NULL, send to the current process.
   If sending to this process, wait for notification that a signal has
   completed before returning.  */
sigset_t __reg3
sig_send (_pinfo *p, siginfo_t& si, _cygtls *tls)
{
  int rc = 1;
  bool its_me;
  HANDLE sendsig;
  sigpacket pack;
  bool communing = si.si_signo == __SIGCOMMUNE;

  pack.wakeup = NULL;
  bool wait_for_completion;
  if (!(its_me = p == NULL || p == myself || p == myself_nowait))
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
      wait_for_completion = p != myself_nowait;
      p = myself;
    }

  /* If myself is the stub process, send signal to the child process
     rather than myself. The fact that myself->dwProcessId is not equal
     to the current process id indicates myself is the stub process. */
  if (its_me && myself->dwProcessId != GetCurrentProcessId ())
    {
      wait_for_completion = false;
      its_me = false;
    }

  if (its_me)
    sendsig = my_sendsig;
  else
    {
      HANDLE dupsig;
      DWORD dwProcessId;
      for (int i = 0; !p->sendsig && i < 10000; i++)
	yield ();
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
      if (!DuplicateHandle (hp, dupsig, GetCurrentProcess (), &sendsig, 0,
			    false, DUPLICATE_SAME_ACCESS) || !sendsig)
	{
	  __seterrno ();
	  sigproc_printf ("DuplicateHandle failed, %E");
	  CloseHandle (hp);
	  goto out;
	}
      VerifyHandle (sendsig);
      if (!communing)
	{
	  CloseHandle (hp);
	  DWORD flag = PIPE_NOWAIT;
	  /* Set PIPE_NOWAIT here to avoid blocking when sending a signal.
	     (Yes, I know MSDN says not to use this)
	     We can't ever block here because it causes a deadlock when
	     debugging with gdb.  */
	  BOOL res = SetNamedPipeHandleState (sendsig, &flag, NULL, NULL);
	  sigproc_printf ("%d = SetNamedPipeHandleState (%y, PIPE_NOWAIT, NULL, NULL)", res, sendsig);
	}
      else
	{
	  si._si_commune._si_process_handle = hp;

	  HANDLE& tome = si._si_commune._si_write_handle;
	  HANDLE& fromthem = si._si_commune._si_read_handle;
	  if (!CreatePipeOverlapped (&fromthem, &tome, &sec_all_nih))
	    {
	      sigproc_printf ("CreatePipe for __SIGCOMMUNE failed, %E");
	      __seterrno ();
	      goto out;
	    }
	  if (!DuplicateHandle (GetCurrentProcess (), tome, hp, &tome, 0, false,
				DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
	    {
	      sigproc_printf ("DuplicateHandle for __SIGCOMMUNE failed, %E");
	      __seterrno ();
	      goto out;
	    }
	}
    }

  sigproc_printf ("sendsig %p, pid %d, signal %d, its_me %d", sendsig, p->pid,
		  si.si_signo, its_me);

  sigset_t pending;
  if (!its_me)
    pack.mask = NULL;
  else if (si.si_signo == __SIGPENDING || si.si_signo == __SIGPENDINGALL)
    pack.mask = &pending;
  else if (si.si_signo == __SIGFLUSH || si.si_signo > 0)
    {
      threadlist_t *tl_entry = cygheap->find_tls (tls ? tls : _main_tls);
      pack.mask = tls ? &tls->sigmask : &_main_tls->sigmask;
      cygheap->unlock_tls (tl_entry);
    }
  else
    pack.mask = NULL;

  pack.si = si;
  if (!pack.si.si_pid)
    pack.si.si_pid = myself->pid;
  if (!pack.si.si_uid)
    pack.si.si_uid = myself->uid;
  pack.pid = myself->pid;
  pack.sigtls = tls;
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
      packsize = sizeof (pack) + sizeof (n) + n;
      char *p = leader = (char *) alloca (packsize);
      memcpy (p, &pack, sizeof (pack)); p += sizeof (pack);
      memcpy (p, &n, sizeof (n)); p += sizeof (n);
      memcpy (p, si._si_commune._si_str, n); p += n;
    }

  DWORD nb;
  BOOL res;
  /* Try multiple times to send if packsize != nb since that probably
     means that the pipe buffer is full.  */
  for (int i = 0; i < 100; i++)
    {
      res = WriteFile (sendsig, leader, packsize, &nb, NULL);
      if (!res || packsize == nb)
	break;
      Sleep (10);
      res = 0;
    }

  if (!res)
    {
      /* Couldn't send to the pipe.  This probably means that the
	 process is exiting.  */
      if (!its_me)
	{
	  sigproc_printf ("WriteFile for pipe %p failed, %E", sendsig);
	  ForceCloseHandle (sendsig);
	}
      else if (!p->exec_sendsig && !exit_state)
	system_printf ("error sending signal %d, pid %u, pipe handle %p, nb %u, packsize %u, %E",
		       si.si_signo, p->pid, sendsig, nb, packsize);
      if (GetLastError () == ERROR_BROKEN_PIPE)
	set_errno (ESRCH);
      else
	__seterrno ();
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
      set_errno (ENOSYS);
      rc = -1;
    }

  if (wait_for_completion && si.si_signo != __SIGFLUSHFAST)
    _my_tls.call_signal_handler ();

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
  if (si.si_signo != __SIGPENDING && si.si_signo != __SIGPENDINGALL)
    /* nothing */;
  else if (!rc)
    rc = pending;
  else
    rc = SIG_BAD_MASK;
  sigproc_printf ("returning %p from sending signal %d", rc, si.si_signo);
  return rc;
}

int child_info::retry_count = 0;

/* Initialize some of the memory block passed to child processes
   by fork/spawn/exec. */
child_info::child_info (unsigned in_cb, child_info_types chtype,
			bool need_subproc_ready):
  msv_count (0), cb (in_cb), intro (PROC_MAGIC_GENERIC),
  magic (CHILD_INFO_MAGIC), type (chtype), cygheap (::cygheap),
  cygheap_max (::cygheap_max), flag (0), retry (child_info::retry_count),
  rd_proc_pipe (NULL), wr_proc_pipe (NULL)
{
  fhandler_union_cb = sizeof (fhandler_union);
  user_h = cygwin_user_h;
  if (strace.active ())
    {
      NTSTATUS status;
      ULONG DebugFlags;

      /* Only propagate _CI_STRACED to child if strace is actually tracing
	 child processes of this process.  The undocumented ProcessDebugFlags
	 returns 0 if EPROCESS->NoDebugInherit is TRUE, 1 otherwise.
	 This avoids a hang when stracing a forking or spawning process
	 with the -f flag set to "don't follow fork". */
      status = NtQueryInformationProcess (GetCurrentProcess (),
					  ProcessDebugFlags, &DebugFlags,
					  sizeof (DebugFlags), NULL);
      if (NT_SUCCESS (status) && DebugFlags)
	flag |= _CI_STRACED;
    }
  if (need_subproc_ready)
    {
      subproc_ready = CreateEvent (&sec_all, FALSE, FALSE, NULL);
      flag |= _CI_ISCYGWIN;
    }
  sigproc_printf ("subproc_ready %p", subproc_ready);
  /* Create an inheritable handle to pass to the child process.  This will
     allow the child to copy cygheap etc. from the parent to itself.  If
     we're forking, we also need handle duplicate access. */
  parent = NULL;
  DWORD perms = PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ
		| PROCESS_VM_OPERATION | SYNCHRONIZE;
  if (type == _CH_FORK)
    {
      perms |= PROCESS_DUP_HANDLE;
      /* VirtualQueryEx is documented to require PROCESS_QUERY_INFORMATION.
	 That's true for Windows 7, but PROCESS_QUERY_LIMITED_INFORMATION
	 appears to be sufficient on Windows 8 and later. */
      if (wincap.needs_query_information ())
	perms |= PROCESS_QUERY_INFORMATION;
    }

  if (!DuplicateHandle (GetCurrentProcess (), GetCurrentProcess (),
			GetCurrentProcess (), &parent, perms, TRUE, 0))
    system_printf ("couldn't create handle to myself for child, %E");
}

child_info::~child_info ()
{
  cleanup ();
}

child_info_fork::child_info_fork () :
  child_info (sizeof *this, _CH_FORK, true),
  forker_finished (NULL)
{
}

child_info_spawn::child_info_spawn (child_info_types chtype, bool need_subproc_ready) :
  child_info (sizeof *this, chtype, need_subproc_ready)
{
  if (type == _CH_EXEC)
    {
      hExeced = NULL;
      if (my_wr_proc_pipe)
	ev = NULL;
      else if (!(ev = CreateEvent (&sec_none_nih, false, false, NULL)))
	api_fatal ("couldn't create signalling event for exec, %E");

      get_proc_lock (PROC_EXECING, 0);
      /* exit with lock held */
    }
}

cygheap_exec_info *
cygheap_exec_info::alloc ()
{
  cygheap_exec_info *res =
    (cygheap_exec_info *) ccalloc_abort (HEAP_1_EXEC, 1,
					 sizeof (cygheap_exec_info)
					 + (chld_procs.count ()
					    * sizeof (children[0])));
  res->sigmask = _my_tls.sigmask;
  return res;
}

void
child_info_spawn::wait_for_myself ()
{
  postfork (myself);
  if (myself.remember ())
    myself.attach ();
  WaitForSingleObject (ev, INFINITE);
}

void
child_info::cleanup ()
{
  if (subproc_ready)
    {
      CloseHandle (subproc_ready);
      subproc_ready = NULL;
    }
  if (parent)
    {
      CloseHandle (parent);
      parent = NULL;
    }
  if (rd_proc_pipe)
    {
      ForceCloseHandle (rd_proc_pipe);
      rd_proc_pipe = NULL;
    }
  if (wr_proc_pipe)
    {
      ForceCloseHandle (wr_proc_pipe);
      wr_proc_pipe = NULL;
    }
}

void
child_info_spawn::cleanup ()
{
  if (moreinfo)
    {
      if (moreinfo->envp)
	{
	  for (char **e = moreinfo->envp; *e; e++)
	    cfree (*e);
	  cfree (moreinfo->envp);
	}
      if (type != _CH_SPAWN && moreinfo->myself_pinfo)
	CloseHandle (moreinfo->myself_pinfo);
      cfree (moreinfo);
    }
  moreinfo = NULL;
  if (ev)
    {
      CloseHandle (ev);
      ev = NULL;
    }
  if (type == _CH_EXEC)
    {
      if (iscygwin () && hExeced)
	proc_subproc (PROC_EXEC_CLEANUP, 0);
      sync_proc_subproc.release ();
    }
  type = _CH_NADA;
  child_info::cleanup ();
}

/* Record any non-reaped subprocesses to be passed to about-to-be-execed
   process.  FIXME: There is a race here if the process exits while we
   are recording it.  */
inline void
cygheap_exec_info::record_children ()
{
  for (nchildren = 0; nchildren < chld_procs.count (); nchildren++)
    {
      children[nchildren].pid = chld_procs[nchildren]->pid;
      children[nchildren].p = chld_procs[nchildren];
      /* Set inheritance of required child handles for reattach_children
	 in the about-to-be-execed process. */
      children[nchildren].p.set_inheritance (true);
    }
}

void
child_info_spawn::record_children ()
{
  if (type == _CH_EXEC && iscygwin ())
    moreinfo->record_children ();
}

/* Reattach non-reaped subprocesses passed in from the cygwin process
   which previously operated under this pid.  FIXME: Is there a race here
   if the process exits during cygwin's exec handoff?  */
inline void
cygheap_exec_info::reattach_children (HANDLE parent)
{
  for (int i = 0; i < nchildren; i++)
    {
      pinfo p (parent, children[i].p, children[i].pid);
      if (!p)
	debug_only_printf ("couldn't reattach child %d from previous process", children[i].pid);
      else if (!p.attach ())
	debug_only_printf ("attach of child process %d failed", children[i].pid);
      else
	debug_only_printf ("reattached pid %d<%u>, process handle %p, rd_proc_pipe %p->%p",
			   p->pid, p->dwProcessId, p.hProcess,
			   children[i].p.rd_proc_pipe, p.rd_proc_pipe);
    }
}

void
child_info_spawn::reattach_children ()
{
  moreinfo->reattach_children (parent);
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
    api_fatal ("SetEvent failed, %E");
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
	  if (type == _CH_EXEC && my_wr_proc_pipe)
	    {
	      ForceCloseHandle1 (hProcess, childhProc);
	      hProcess = NULL;
	    }
	}
      sigproc_printf ("pid %u, WFMO returned %d, exit_code %y, res %d", pid, x,
		      exit_code, res);
    }
  return res;
}

DWORD
child_info::proc_retry (HANDLE h)
{
  if (!exit_code)
    return EXITCODE_OK;
  sigproc_printf ("exit_code %y", exit_code);
  switch (exit_code)
    {
    case STILL_ACTIVE:	/* shouldn't happen */
      sigproc_printf ("STILL_ACTIVE?  How'd we get here?");
      break;
    case STATUS_DLL_NOT_FOUND:
    case STATUS_ACCESS_VIOLATION:
    case STATUS_ILLEGAL_INSTRUCTION:
    case STATUS_ILLEGAL_DLL_PSEUDO_RELOCATION: /* pseudo-reloc.c specific */
      return exit_code;
    case STATUS_CONTROL_C_EXIT:
      if (saw_ctrl_c ())
	return EXITCODE_OK;
      fallthrough;
    case STATUS_DLL_INIT_FAILED:
    case STATUS_DLL_INIT_FAILED_LOGOFF:
    case EXITCODE_RETRY:
      if (retry-- > 0)
	exit_code = 0;
      break;
    case EXITCODE_FORK_FAILED: /* windows prevented us from forking */
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
child_info_fork::abort (const char *fmt, ...)
{
  if (fmt)
    {
      va_list ap;
      va_start (ap, fmt);
      if (silentfail ())
	strace_vprintf (DEBUG, fmt, ap);
      else
	strace_vprintf (SYSTEM, fmt, ap);
      TerminateProcess (GetCurrentProcess (), EXITCODE_FORK_FAILED);
    }
  if (retry > 0)
    TerminateProcess (GetCurrentProcess (), EXITCODE_RETRY);
  return false;
}

/* Check the state of all of our children to see if any are stopped or
 * terminated.
 */
static int __reg1
checkstate (waitq *parent_w)
{
  int potential_match = 0;

  sigproc_printf ("child_procs count %d", chld_procs.count ());

  /* Check already dead processes first to see if they match the criteria
   * given in w->next.  */
  int res;
  for (int i = 0; i < chld_procs.count (); i++)
    if ((res = stopped_or_terminated (parent_w, chld_procs[i])))
      {
	remove_proc (i);
	potential_match = 1;
	goto out;
      }

  sigproc_printf ("no matching terminated children found");
  potential_match = -!!chld_procs.count ();

out:
  sigproc_printf ("returning %d", potential_match);
  return potential_match;
}

/* Remove a proc from chld_procs by swapping it with the last child in the list.
   Also releases shared memory of exited processes.  */
static int
remove_proc (int ci)
{
  if (have_execed)
    {
      if (_my_tls._ctinfo != chld_procs[ci].wait_thread)
	chld_procs[ci].wait_thread->terminate_thread ();
    }
  else if (chld_procs[ci] && chld_procs[ci]->exists ())
    return 1;

  sigproc_printf ("removing chld_procs[%d], pid %d, child_procs count %d",
		  ci, chld_procs[ci]->pid, chld_procs.count ());
  if (chld_procs[ci] != myself)
    chld_procs[ci].release ();
  if (ci < chld_procs.del_one ())
    {
      /* Wait for proc_waiter thread to make a copy of this element before
	 moving it or it may become confused.  The chances are very high that
	 the proc_waiter thread has already done this by the time we
	 get here.  */
      if (!have_execed && !exit_state)
	while (!chld_procs[chld_procs.count ()].waiter_ready)
	  yield ();
      chld_procs[ci] = chld_procs[chld_procs.count ()];
    }
  return 0;
}

/* Check status of child process vs. waitq member.

   parent_w is the pointer to the parent of the waitq member in question.
   child is the subprocess being considered.

   Returns non-zero if waiting thread released.  */
static bool
stopped_or_terminated (waitq *parent_w, _pinfo *child)
{
  int might_match;
  waitq *w = parent_w->next;

  sigproc_printf ("considering pid %d, pgid %d, w->pid %d", child->pid, child->pgid, w->pid);
  if (w->pid == -1)
    might_match = 1;
  else if (w->pid == 0)
    might_match = child->pgid == myself->pgid;
  else if (w->pid < 0)
    might_match = child->pgid == -w->pid;
  else
    might_match = (w->pid == child->pid);

  if (!might_match)
    return false;

  int terminated;

  if (!((terminated = (child->process_state == PID_EXITED))
	|| ((w->options & WCONTINUED) && child->stopsig == SIGCONT)
	|| ((w->options & WUNTRACED) && child->stopsig && child->stopsig != SIGCONT)))
    return false;

  parent_w->next = w->next;	/* successful wait.  remove from wait queue */
  w->pid = child->pid;

  if (!terminated)
    {
      sigproc_printf ("stopped child, stop signal %d", child->stopsig);
      if (child->stopsig == SIGCONT)
	w->status = __W_CONTINUED;
      else
	w->status = (child->stopsig << 8) | 0x7f;
      child->stopsig = 0;
    }
  else
    {
      child->process_state = PID_REAPED;
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
    new cygthread (commune_process, size, si, "commune");
}

/* Add a packet to the beginning of the queue.
   Should only be called from signal thread.  */
void
pending_signals::add (sigpacket& pack)
{
  sigpacket *se;

  se = sigs + pack.si.si_signo;
  if (se->si.si_signo)
    return;
  *se = pack;
  se->next = start.next;
  start.next = se;
}

/* Process signals by waiting for signal data to arrive in a pipe.
   Set a completion event if one was specified. */
static void WINAPI
wait_sig (VOID *)
{
  _sig_tls = &_my_tls;
  bool sig_held = false;

  sigproc_printf ("entering ReadFile loop, my_readsig %p, my_sendsig %p",
		  my_readsig, my_sendsig);

  hntdll = GetModuleHandle ("ntdll.dll");

  for (;;)
    {
      DWORD nb;
      sigpacket pack = {};
      if (sigq.retry)
	pack.si.si_signo = __SIGFLUSH;
      else if (!ReadFile (my_readsig, &pack, sizeof (pack), &nb, NULL))
	Sleep (INFINITE);	/* Assume were exiting.  Never exit this thread */
      else if (nb != sizeof (pack) || !pack.si.si_signo)
	{
	  system_printf ("garbled signal pipe data nb %u, sig %d", nb, pack.si.si_signo);
	  continue;
	}

      sigq.retry = false;
      /* Don't process signals when we start exiting */
      if (exit_state > ES_EXIT_STARTING && pack.si.si_signo > 0)
	goto skip_process_signal;

      sigset_t dummy_mask;
      threadlist_t *tl_entry;
      if (!pack.mask)
	{
	  tl_entry = cygheap->find_tls (_main_tls);
	  dummy_mask = _main_tls->sigmask;
	  cygheap->unlock_tls (tl_entry);
	  pack.mask = &dummy_mask;
	}

      sigpacket *q;
      q = &sigq.start;
      bool clearwait;
      clearwait = false;
      switch (pack.si.si_signo)
	{
	case __SIGCOMMUNE:
	  talktome (&pack.si);
	  break;
	case __SIGSTRACE:
	  strace.activate (false);
	  break;
	case __SIGPENDINGALL:
	  {
	    unsigned bit;
	    bool issig_wait;

	    *pack.mask = 0;
	    while ((q = q->next))
	      {
		if (q->sigtls->sigmask & (bit = SIGTOMASK (q->si.si_signo)))
		  {
		    tl_entry = cygheap->find_tls (q->si.si_signo, issig_wait);
		    if (tl_entry)
		      {
			*pack.mask |= bit;
			cygheap->unlock_tls (tl_entry);
		      }
		  }
	      }
	  }
	  break;
	case __SIGPENDING:
	  {
	    unsigned bit;

	    *pack.mask = 0;
	    tl_entry = cygheap->find_tls (pack.sigtls);
	    while ((q = q->next))
	      {
		/* Skip thread-specific signals for other threads. */
		if (q->sigtls && pack.sigtls != q->sigtls)
		  continue;
		if (pack.sigtls->sigmask & (bit = SIGTOMASK (q->si.si_signo)))
		  *pack.mask |= bit;
	      }
	    cygheap->unlock_tls (tl_entry);
	  }
	  break;
	case __SIGHOLD:
	  sig_held = true;
	  break;
	case __SIGSETPGRP:
	  init_console_handler (::cygheap->ctty
				&& ::cygheap->ctty->is_console ());
	  break;
	case __SIGTHREADEXIT:
	  {
	    /* Serialize thread exit as the thread exit code can be interpreted
	       as the process exit code in some cases when racing with
	       ExitProcess/TerminateProcess.
	       So, wait for the thread which sent this signal to exit, then
	       release the process lock which it held and close it's handle.
	       See cgf-000017 in DevNotes for more details.
	       */
	    HANDLE h = (HANDLE) pack.si.si_cyg;
	    DWORD res = WaitForSingleObject (h, 5000);
	    lock_process::force_release (pack.sigtls);
	    ForceCloseHandle1 (h, exit_thread);
	    if (res != WAIT_OBJECT_0)
	      {
#ifdef DEBUGGING
		try_to_debug();
#endif
		system_printf ("WaitForSingleObject(%p) for thread exit returned %u", h, res);
	      }
	  }
	  break;
	default:	/* Normal (positive) signal */
	  if (pack.si.si_signo < 0)
	    sig_clear (-pack.si.si_signo);
	  else
	    sigq.add (pack);
	  fallthrough;
	case __SIGNOHOLD:
	  sig_held = false;
	  fallthrough;
	case __SIGFLUSH:
	case __SIGFLUSHFAST:
	  if (!sig_held)
	    {
	      sigpacket *qnext;
	      /* Check the queue for signals.  There will always be at least one
		 thing on the queue if this was a valid signal.  */
	      while ((qnext = q->next))
		{
		  if (qnext->si.si_signo && qnext->process () <= 0)
		    q = qnext;
		  else
		    {
		      q->next = qnext->next;
		      qnext->si.si_signo = 0;
		    }
		}
	      /* At least one signal still queued?  The event is used in select
		 only, and only to decide if WFMO should wake up in case a
		 signalfd is waiting via select/poll for being ready to read a
		 pending signal.  This method wakes up all threads hanging in
		 select and having a signalfd, as soon as a pending signal is
		 available, but it's certainly better than constant polling. */
	      if (sigq.start.next)
		SetEvent (my_pendingsigs_evt);
	      else
		ResetEvent (my_pendingsigs_evt);
	      if (pack.si.si_signo == SIGCHLD)
		clearwait = true;
	    }
	  break;
	case __SIGNONCYGCHLD:
	  cygheap_fdenum cfd (false);
	  while (cfd.next () >= 0)
	    if (cfd->get_dev () == FH_PIPEW)
	      {
		fhandler_pipe *pipe = (fhandler_pipe *)(fhandler_base *) cfd;
		if (pipe->need_close_query_hdl ())
		  pipe->close_query_handle ();
	      }
	  break;
	}
      if (clearwait && !have_execed)
	proc_subproc (PROC_CLEARWAIT, 0);
skip_process_signal:
      if (pack.wakeup)
	{
	  sigproc_printf ("signalling pack.wakeup %p", pack.wakeup);
	  SetEvent (pack.wakeup);
	}
    }
}
