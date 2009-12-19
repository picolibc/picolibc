/* signal.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009 Red Hat, Inc.

   Written by Steve Chamberlain of Cygnus Support, sac@cygnus.com
   Significant changes by Sergey Okhapkin <sos@prospect.com.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <sys/cygwin.h>
#include "pinfo.h"
#include "sigproc.h"
#include "cygtls.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"

int sigcatchers;	/* FIXME: Not thread safe. */

#define _SA_NORESTART	0x8000

static int sigaction_worker (int, const struct sigaction *, struct sigaction *, bool)
  __attribute__ ((regparm (3)));

#define sigtrapped(func) ((func) != SIG_IGN && (func) != SIG_DFL)

static inline void
set_sigcatchers (void (*oldsig) (int), void (*cursig) (int))
{
#ifdef DEBUGGING
  int last_sigcatchers = sigcatchers;
#endif
  if (!sigtrapped (oldsig) && sigtrapped (cursig))
    sigcatchers++;
  else if (sigtrapped (oldsig) && !sigtrapped (cursig))
    sigcatchers--;
#ifdef DEBUGGING
  if (last_sigcatchers != sigcatchers)
    sigproc_printf ("last %d, old %d, cur %p, cur %p", last_sigcatchers,
		    sigcatchers, oldsig, cursig);
#endif
}

extern "C" _sig_func_ptr
signal (int sig, _sig_func_ptr func)
{
  sig_dispatch_pending ();
  _sig_func_ptr prev;

  /* check that sig is in right range */
  if (sig < 0 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP)
    {
      set_errno (EINVAL);
      syscall_printf ("SIG_ERR = signal (%d, %p)", sig, func);
      return (_sig_func_ptr) SIG_ERR;
    }

  prev = global_sigs[sig].sa_handler;
  struct sigaction& gs = global_sigs[sig];
  if (gs.sa_flags & _SA_NORESTART)
    gs.sa_flags &= ~SA_RESTART;
  else
    gs.sa_flags |= SA_RESTART;

  gs.sa_mask = SIGTOMASK (sig);
  gs.sa_handler = func;
  gs.sa_flags &= ~SA_SIGINFO;

  set_sigcatchers (prev, func);

  syscall_printf ("%p = signal (%d, %p)", prev, sig, func);
  return prev;
}

extern "C" int
nanosleep (const struct timespec *rqtp, struct timespec *rmtp)
{
  int res = 0;
  sig_dispatch_pending ();
  pthread_testcancel ();

  if ((unsigned int) rqtp->tv_nsec > 999999999)
    {
      set_errno (EINVAL);
      return -1;
    }
  unsigned int sec = rqtp->tv_sec;
  DWORD resolution = gtod.resolution ();
  bool done = false;
  DWORD req;
  DWORD rem;

  while (!done)
    {
      /* Divide user's input into transactions no larger than 49.7
         days at a time.  */
      if (sec > HIRES_DELAY_MAX / 1000)
        {
          req = ((HIRES_DELAY_MAX + resolution - 1)
                 / resolution * resolution);
          sec -= HIRES_DELAY_MAX / 1000;
        }
      else
        {
          req = ((sec * 1000 + (rqtp->tv_nsec + 999999) / 1000000
                  + resolution - 1) / resolution) * resolution;
          sec = 0;
          done = true;
        }

      DWORD end_time = gtod.dmsecs () + req;
      syscall_printf ("nanosleep (%ld)", req);

      int rc = cancelable_wait (signal_arrived, req);
      if ((rem = end_time - gtod.dmsecs ()) > HIRES_DELAY_MAX)
        rem = 0;
      if (rc == WAIT_OBJECT_0)
        {
          _my_tls.call_signal_handler ();
          set_errno (EINTR);
          res = -1;
          break;
        }
    }

  if (rmtp)
    {
      rmtp->tv_sec = sec + rem / 1000;
      rmtp->tv_nsec = (rem % 1000) * 1000000;
      if (sec)
        {
          rmtp->tv_nsec += rqtp->tv_nsec;
          if (rmtp->tv_nsec >= 1000000000)
            {
              rmtp->tv_nsec -= 1000000000;
              rmtp->tv_sec++;
            }
        }
    }

  syscall_printf ("%d = nanosleep (%ld, %ld)", res, req, rem);
  return res;
}

extern "C" unsigned int
sleep (unsigned int seconds)
{
  struct timespec req, rem;
  req.tv_sec = seconds;
  req.tv_nsec = 0;
  if (nanosleep (&req, &rem))
    return rem.tv_sec + (rem.tv_nsec > 0);
  return 0;
}

extern "C" unsigned int
usleep (useconds_t useconds)
{
  struct timespec req;
  req.tv_sec = useconds / 1000000;
  req.tv_nsec = (useconds % 1000000) * 1000;
  int res = nanosleep (&req, NULL);
  return res;
}

extern "C" int
sigprocmask (int how, const sigset_t *set, sigset_t *oldset)
{
  return handle_sigprocmask (how, set, oldset, _my_tls.sigmask);
}

int __stdcall
handle_sigprocmask (int how, const sigset_t *set, sigset_t *oldset, sigset_t& opmask)
{
  /* check that how is in right range */
  if (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK)
    {
      syscall_printf ("Invalid how value %d", how);
      set_errno (EINVAL);
      return -1;
    }

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  if (oldset)
    *oldset = opmask;

  if (set)
    {
      sigset_t newmask = opmask;
      switch (how)
	{
	case SIG_BLOCK:
	  /* add set to current mask */
	  newmask |= *set;
	  break;
	case SIG_UNBLOCK:
	  /* remove set from current mask */
	  newmask &= ~*set;
	  break;
	case SIG_SETMASK:
	  /* just set it */
	  newmask = *set;
	  break;
	}
      set_signal_mask (newmask, opmask);
    }
  return 0;
}

int __stdcall
_pinfo::kill (siginfo_t& si)
{
  sig_dispatch_pending ();

  int res = 0;
  bool sendSIGCONT;

  if (!exists ())
    {
      set_errno (ESRCH);
      return -1;
    }

  if ((sendSIGCONT = (si.si_signo < 0)))
    si.si_signo = -si.si_signo;

  DWORD this_process_state = process_state;
  if (si.si_signo == 0)
    /* ok */;
  else if ((res = sig_send (this, si)))
    {
      sigproc_printf ("%d = sig_send, %E ", res);
      res = -1;
    }
  else if (sendSIGCONT)
    {
      siginfo_t si2 = {0};
      si2.si_signo = SIGCONT;
      si2.si_code = SI_KERNEL;
      sig_send (this, si2);
    }

  syscall_printf ("%d = _pinfo::kill (%d, %d), process_state %p", res, pid,
		  si.si_signo, this_process_state);
  return res;
}

int
raise (int sig)
{
  return kill (myself->pid, sig);
}

static int
kill0 (pid_t pid, siginfo_t& si)
{
  syscall_printf ("kill (%d, %d)", pid, si.si_signo);
  /* check that sig is in right range */
  if (si.si_signo < 0 || si.si_signo >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("signal %d out of range", si.si_signo);
      return -1;
    }

  /* Silently ignore stop signals from a member of orphaned process group.
     FIXME: Why??? */
  if (ISSTATE (myself, PID_ORPHANED) &&
      (si.si_signo == SIGTSTP || si.si_signo == SIGTTIN || si.si_signo == SIGTTOU))
    si.si_signo = 0;

  return (pid > 0) ? pinfo (pid)->kill (si) : kill_pgrp (-pid, si);
}

int
killsys (pid_t pid, int sig)
{
  siginfo_t si = {0};
  si.si_signo = sig;
  si.si_code = SI_KERNEL;
  return kill0 (pid, si);
}

int
kill (pid_t pid, int sig)
{
  siginfo_t si = {0};
  si.si_signo = sig;
  si.si_code = SI_USER;
  return kill0 (pid, si);
}

int
kill_pgrp (pid_t pid, siginfo_t& si)
{
  int res = 0;
  int found = 0;
  int killself = 0;

  sigproc_printf ("pid %d, signal %d", pid, si.si_signo);

  winpids pids ((DWORD) PID_MAP_RW);
  for (unsigned i = 0; i < pids.npids; i++)
    {
      _pinfo *p = pids[i];

      if (!p->exists ())
	continue;

      /* Is it a process we want to kill?  */
      if ((pid == 0 && (p->pgid != myself->pgid || p->ctty != myself->ctty)) ||
	  (pid > 1 && p->pgid != pid) ||
	  (si.si_signo < 0 && NOTSTATE (p, PID_STOPPED)))
	continue;
      sigproc_printf ("killing pid %d, pgrp %d, p->%s, %s", p->pid, p->pgid,
		      p->__ctty (), myctty ());
      if (p == myself)
	killself++;
      else if (p->kill (si))
	res = -1;
      found++;
    }

  if (killself && !exit_state && myself->kill (si))
    res = -1;

  if (!found)
    {
      set_errno (ESRCH);
      res = -1;
    }
  syscall_printf ("%d = kill (%d, %d)", res, pid, si.si_signo);
  return res;
}

extern "C" int
killpg (pid_t pgrp, int sig)
{
  return kill (-pgrp, sig);
}

extern "C" void
abort (void)
{
  _my_tls.incyg++;
  sig_dispatch_pending ();
  /* Ensure that SIGABRT can be caught regardless of blockage. */
  sigset_t sig_mask;
  sigfillset (&sig_mask);
  sigdelset (&sig_mask, SIGABRT);
  set_signal_mask (sig_mask, _my_tls.sigmask);

  raise (SIGABRT);
  _my_tls.call_signal_handler (); /* Call any signal handler */

  /* Flush all streams as per SUSv2.  */
  if (_GLOBAL_REENT->__cleanup)
    _GLOBAL_REENT->__cleanup (_GLOBAL_REENT);
  do_exit (SIGABRT);	/* signal handler didn't exit.  Goodbye. */
}

static int
sigaction_worker (int sig, const struct sigaction *newact, struct sigaction *oldact, bool isinternal)
{
  sig_dispatch_pending ();
  /* check that sig is in right range */
  if (sig < 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      sigproc_printf ("signal %d, newact %p, oldact %p", sig, newact, oldact);
      syscall_printf ("SIG_ERR = sigaction signal %d out of range", sig);
      return -1;
    }

  struct sigaction oa = global_sigs[sig];

  if (!newact)
    sigproc_printf ("signal %d, newact %p, oa %p", sig, newact, oa, oa.sa_handler);
  else
    {
      sigproc_printf ("signal %d, newact %p (handler %p), oa %p", sig, newact, newact->sa_handler, oa, oa.sa_handler);
      if (sig == SIGKILL || sig == SIGSTOP)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      struct sigaction na = *newact;
      struct sigaction& gs = global_sigs[sig];
      if (!isinternal)
	na.sa_flags &= ~_SA_INTERNAL_MASK;
      gs = na;
      if (!(gs.sa_flags & SA_NODEFER))
	gs.sa_mask |= SIGTOMASK(sig);
      if (gs.sa_handler == SIG_IGN)
	sig_clear (sig);
      if (gs.sa_handler == SIG_DFL && sig == SIGCHLD)
	sig_clear (sig);
      set_sigcatchers (oa.sa_handler, gs.sa_handler);
      if (sig == SIGCHLD)
	{
	  myself->process_state &= ~PID_NOCLDSTOP;
	  if (gs.sa_flags & SA_NOCLDSTOP)
	    myself->process_state |= PID_NOCLDSTOP;
	}
    }

  if (oldact)
    {
      *oldact = oa;
      oa.sa_flags &= ~_SA_INTERNAL_MASK;
    }

  return 0;
}

extern "C" int
sigaction (int sig, const struct sigaction *newact, struct sigaction *oldact)
{
  return sigaction_worker (sig, newact, oldact, false);
}

extern "C" int
sigaddset (sigset_t *set, const int sig)
{
  /* check that sig is in right range */
  if (sig <= 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("SIG_ERR = sigaddset signal %d out of range", sig);
      return -1;
    }

  *set |= SIGTOMASK (sig);
  return 0;
}

extern "C" int
sigdelset (sigset_t *set, const int sig)
{
  /* check that sig is in right range */
  if (sig <= 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("SIG_ERR = sigdelset signal %d out of range", sig);
      return -1;
    }

  *set &= ~SIGTOMASK (sig);
  return 0;
}

extern "C" int
sigismember (const sigset_t *set, int sig)
{
  /* check that sig is in right range */
  if (sig <= 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("SIG_ERR = sigdelset signal %d out of range", sig);
      return -1;
    }

  if (*set & SIGTOMASK (sig))
    return 1;
  else
    return 0;
}

extern "C" int
sigemptyset (sigset_t *set)
{
  *set = (sigset_t) 0;
  return 0;
}

extern "C" int
sigfillset (sigset_t *set)
{
  *set = ~((sigset_t) 0);
  return 0;
}

extern "C" int
sigsuspend (const sigset_t *set)
{
  return handle_sigsuspend (*set);
}

extern "C" int
sigpause (int signal_mask)
{
  return handle_sigsuspend ((sigset_t) signal_mask);
}

extern "C" int
pause (void)
{
  return handle_sigsuspend (_my_tls.sigmask);
}

extern "C" int
siginterrupt (int sig, int flag)
{
  struct sigaction act;
  sigaction (sig, NULL, &act);
  if (flag)
    {
      act.sa_flags &= ~SA_RESTART;
      act.sa_flags |= _SA_NORESTART;
    }
  else
    {
      act.sa_flags &= ~_SA_NORESTART;
      act.sa_flags |= SA_RESTART;
    }
  return sigaction_worker (sig, &act, NULL, true);
}

extern "C" int
sigwait (const sigset_t *set, int *sig_ptr)
{
  int sig = sigwaitinfo (set, NULL);
  if (sig > 0)
    *sig_ptr = sig;
  return sig > 0 ? 0 : -1;
}

extern "C" int
sigwaitinfo (const sigset_t *set, siginfo_t *info)
{
  pthread_testcancel ();
  HANDLE h;
  h = _my_tls.event = CreateEvent (&sec_none_nih, FALSE, FALSE, NULL);
  if (!h)
    {
      __seterrno ();
      return -1;
    }

  _my_tls.sigwait_mask = *set;
  sig_dispatch_pending (true);

  int res;
  switch (WaitForSingleObject (h, INFINITE))
    {
    case WAIT_OBJECT_0:
      if (!sigismember (set, _my_tls.infodata.si_signo))
	{
	  set_errno (EINTR);
	  res = -1;
	}
      else
	{
	  if (info)
	    *info = _my_tls.infodata;
	  res = _my_tls.infodata.si_signo;
	  InterlockedExchange ((LONG *) &_my_tls.sig, (LONG) 0);
	}
      break;
    default:
      __seterrno ();
      res = -1;
    }
  CloseHandle (h);
  sigproc_printf ("returning sig %d", res);
  return res;
}

/* FIXME: SUSv3 says that this function should block until the signal has
   actually been delivered.  Currently, this will only happen when sending
   signals to the current process.  It will not happen when sending signals
   to other processes.  */
extern "C" int
sigqueue (pid_t pid, int sig, const union sigval value)
{
  siginfo_t si = {0};
  pinfo dest (pid);
  if (!dest)
    {
      set_errno (ESRCH);
      return -1;
    }
  si.si_signo = sig;
  si.si_code = SI_QUEUE;
  si.si_value = value;
  return sig_send (dest, si);
}
