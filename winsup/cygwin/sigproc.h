/* sigproc.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once
#include <signal.h>
#include "sync.h"

#ifdef NSIG
enum
{
  __SIGFLUSH	    = -(NSIG + 1),
  __SIGSTRACE	    = -(NSIG + 2),
  __SIGCOMMUNE	    = -(NSIG + 3),
  __SIGPENDING	    = -(NSIG + 4),
  __SIGDELETE	    = -(NSIG + 5),	/* Not currently used */
  __SIGFLUSHFAST    = -(NSIG + 6),
  __SIGHOLD	    = -(NSIG + 7),
  __SIGNOHOLD	    = -(NSIG + 8),
  __SIGSETPGRP	    = -(NSIG + 9),
  __SIGTHREADEXIT   = -(NSIG + 10)
};
#endif

#define SIG_BAD_MASK (1 << (SIGKILL - 1))

enum procstuff
{
  PROC_ADDCHILD		  = 1,	// add a new subprocess to list
  PROC_REATTACH_CHILD	  = 2,	// reattach after exec
  PROC_EXEC_CLEANUP	  = 3,	// cleanup waiting children after exec
  PROC_DETACHED_CHILD	  = 4,	// set up a detached child
  PROC_CLEARWAIT	  = 5,	// clear all waits - signal arrived
  PROC_WAIT		  = 6,	// setup for wait() for subproc
  PROC_EXECING		  = 7,	// used to get a lock when execing
  PROC_NOTHING		  = 8	// nothing, really
};

struct sigpacket
{
  siginfo_t si;
  pid_t pid;
  class _cygtls *sigtls;
  sigset_t *mask;
  union
  {
    HANDLE wakeup;
    HANDLE thread_handle;
    struct sigpacket *next;
  };
  int __reg1 process ();
  int __reg3 setup_handler (void *, struct sigaction&, _cygtls *);
};

void __reg1 sig_dispatch_pending (bool fast = false);
void __reg2 set_signal_mask (sigset_t&, sigset_t);
int __reg3 handle_sigprocmask (int sig, const sigset_t *set,
				  sigset_t *oldset, sigset_t& opmask);

void __reg1 sig_clear (int);
void __reg1 sig_set_pending (int);
int __stdcall handle_sigsuspend (sigset_t);

int __reg2 proc_subproc (DWORD, uintptr_t);

class _pinfo;
void __stdcall proc_terminate ();
void __stdcall sigproc_init ();
bool __reg1 pid_exists (pid_t);
int __reg3 sig_send (_pinfo *, siginfo_t&, class _cygtls * = NULL);
int __reg3 sig_send (_pinfo *, int, class _cygtls * = NULL);
void __stdcall signal_fixup_after_exec ();
void __stdcall sigalloc ();

int kill_pgrp (pid_t, siginfo_t&);
void __reg1 exit_thread (DWORD) __attribute__ ((noreturn));
void __reg1 setup_signal_exit (int);

class no_thread_exit_protect
{
  static bool flag;
  bool modify;
public:
  no_thread_exit_protect (int) {flag = true; modify = true;}
  ~no_thread_exit_protect ()
  {
    if (modify)
      flag = false;
  }
  no_thread_exit_protect () {modify = false;}
  operator int () {return flag;}
};


extern "C" void sigdelayed ();

extern char myself_nowait_dummy[];

extern struct sigaction *global_sigs;

class lock_signals
{
  bool worked;
public:
  lock_signals ()
  {
    worked = sig_send (NULL, __SIGHOLD) == 0;
  }
  operator int () const
  {
    return worked;
  }
  void dont_bother ()
  {
    worked = false;
  }
  ~lock_signals ()
  {
    if (worked)
      sig_send (NULL, __SIGNOHOLD);
  }
};

class lock_pthread
{
  bool bother;
public:
  lock_pthread (): bother (1)
  {
    pthread::atforkprepare ();
  }
  void dont_bother ()
  {
    bother = false;
  }
  ~lock_pthread ()
  {
    if (bother)
      pthread::atforkparent ();
  }
};

class hold_everything
{
  bool& ischild;
  /* Note the order of the locks below.  It is important,
     to avoid races, that the lock order be preserved.

     pthread is first because it serves as a master lock
     against other forks being attempted while this one is active.

     signals is next to stop signal processing for the duration
     of the fork.

     process is last.  If it is put before signals, then a deadlock
     could be introduced if the process attempts to exit due to a signal. */
  lock_pthread pthread;
  lock_signals signals;
  lock_process process;

public:
  hold_everything (bool& x): ischild (x) {}
  operator int () const {return signals;}

  ~hold_everything()
  {
    if (ischild)
      {
	pthread.dont_bother ();
	process.dont_bother ();
	signals.dont_bother ();
      }
  }

};

#define myself_nowait ((_pinfo *) myself_nowait_dummy)
