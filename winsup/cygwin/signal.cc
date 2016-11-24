/* signal.cc

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
#include "cygwait.h"

#define _SA_NORESTART	0x8000

static int __reg3 sigaction_worker (int, const struct sigaction *, struct sigaction *, bool);

#define sigtrapped(func) ((func) != SIG_IGN && (func) != SIG_DFL)

extern "C" _sig_func_ptr
signal (int sig, _sig_func_ptr func)
{
  sig_dispatch_pending ();
  _sig_func_ptr prev;

  /* check that sig is in right range */
  if (sig <= 0 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP)
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

  syscall_printf ("%p = signal (%d, %p)", prev, sig, func);
  return prev;
}

extern "C" int
clock_nanosleep (clockid_t clk_id, int flags, const struct timespec *rqtp,
		 struct timespec *rmtp)
{
  const bool abstime = (flags & TIMER_ABSTIME) ? true : false;
  int res = 0;
  sig_dispatch_pending ();
  pthread_testcancel ();

  if (rqtp->tv_sec < 0 || rqtp->tv_nsec < 0 || rqtp->tv_nsec > 999999999L)
    return EINVAL;

  /* Explicitly disallowed by POSIX. Needs to be checked first to avoid
     being caught by the following test. */
  if (clk_id == CLOCK_THREAD_CPUTIME_ID)
    return EINVAL;

  /* support for CPU-time clocks is optional */
  if (CLOCKID_IS_PROCESS (clk_id) || CLOCKID_IS_THREAD (clk_id))
    return ENOTSUP;

  switch (clk_id)
    {
    case CLOCK_REALTIME:
    case CLOCK_MONOTONIC:
      break;
    default:
      /* unknown or illegal clock ID */
      return EINVAL;
    }

  LARGE_INTEGER timeout;

  timeout.QuadPart = (LONGLONG) rqtp->tv_sec * NSPERSEC
		     + ((LONGLONG) rqtp->tv_nsec + 99LL) / 100LL;

  if (abstime)
    {
      struct timespec tp;

      clock_gettime (clk_id, &tp);
      /* Check for immediate timeout */
      if (tp.tv_sec > rqtp->tv_sec
	  || (tp.tv_sec == rqtp->tv_sec && tp.tv_nsec > rqtp->tv_nsec))
	return 0;

      if (clk_id == CLOCK_REALTIME)
	timeout.QuadPart += FACTOR;
      else
	{
	  /* other clocks need to be handled with a relative timeout */
	  timeout.QuadPart -= tp.tv_sec * NSPERSEC + tp.tv_nsec / 100LL;
	  timeout.QuadPart *= -1LL;
	}
    }
  else /* !abstime */
    timeout.QuadPart *= -1LL;

  syscall_printf ("clock_nanosleep (%ld.%09ld)", rqtp->tv_sec, rqtp->tv_nsec);

  int rc = cygwait (NULL, &timeout, cw_sig_eintr | cw_cancel | cw_cancel_self);
  if (rc == WAIT_SIGNALED)
    res = EINTR;

  /* according to POSIX, rmtp is used only if !abstime */
  if (rmtp && !abstime)
    {
      rmtp->tv_sec = (time_t) (timeout.QuadPart / NSPERSEC);
      rmtp->tv_nsec = (long) ((timeout.QuadPart % NSPERSEC) * 100LL);
    }

  syscall_printf ("%d = clock_nanosleep(%lu, %d, %ld.%09ld, %ld.%09.ld)",
		  res, clk_id, flags, rqtp->tv_sec, rqtp->tv_nsec,
		  rmtp ? rmtp->tv_sec : 0, rmtp ? rmtp->tv_nsec : 0);
  return res;
}

extern "C" int
nanosleep (const struct timespec *rqtp, struct timespec *rmtp)
{
  int res = clock_nanosleep (CLOCK_REALTIME, 0, rqtp, rmtp);
  if (res != 0)
    {
      set_errno (res);
      return -1;
    }
  return 0;
}

extern "C" unsigned int
sleep (unsigned int seconds)
{
  struct timespec req, rem;
  req.tv_sec = seconds;
  req.tv_nsec = 0;
  if (clock_nanosleep (CLOCK_REALTIME, 0, &req, &rem))
    return rem.tv_sec + (rem.tv_nsec > 0);
  return 0;
}

extern "C" unsigned int
usleep (useconds_t useconds)
{
  struct timespec req;
  req.tv_sec = useconds / 1000000;
  req.tv_nsec = (useconds % 1000000) * 1000;
  int res = clock_nanosleep (CLOCK_REALTIME, 0, &req, NULL);
  if (res != 0)
    {
      set_errno (res);
      return -1;
    }
  return 0;
}

extern "C" int
sigprocmask (int how, const sigset_t *set, sigset_t *oldset)
{
  int res = handle_sigprocmask (how, set, oldset, _my_tls.sigmask);
  if (res)
    {
      set_errno (res);
      res = -1;
    }
  syscall_printf ("%R = sigprocmask (%d, %p, %p)", res, how, set, oldset);
  return res;
}

int __reg3
handle_sigprocmask (int how, const sigset_t *set, sigset_t *oldset, sigset_t& opmask)
{
  /* check that how is in right range */
  if (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK)
    {
      syscall_printf ("Invalid how value %d", how);
      return EINVAL;
    }

  __try
	{
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
	  set_signal_mask (opmask, newmask);
	}
    }
  __except (EFAULT)
    {
      return EFAULT;
    }
  __endtry
  return 0;
}

int __reg2
_pinfo::kill (siginfo_t& si)
{
  int res;
  DWORD this_process_state;
  pid_t this_pid;

  sig_dispatch_pending ();

  if (exists ())
    {
      bool sendSIGCONT;
      this_process_state = process_state;
      if ((sendSIGCONT = (si.si_signo < 0)))
	si.si_signo = -si.si_signo;

      if (si.si_signo == 0)
	res = 0;
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
      this_pid = pid;
    }
  else if (this && process_state == PID_EXITED)
    {
      this_process_state = process_state;
      this_pid = pid;
      res = 0;
    }
  else
    {
      set_errno (ESRCH);
      this_process_state = 0;
      this_pid = 0;
      res = -1;
    }

  syscall_printf ("%d = _pinfo::kill (%d), pid %d, process_state %y", res,
		  si.si_signo, this_pid, this_process_state);
  return res;
}

extern "C" int
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

  return (pid > 0) ? pinfo (pid)->kill (si) : kill_pgrp (-pid, si);
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
  syscall_printf ("%R = kill(%d, %d)", res, pid, si.si_signo);
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
  set_signal_mask (_my_tls.sigmask, sig_mask);

  raise (SIGABRT);
  _my_tls.call_signal_handler (); /* Call any signal handler */

  /* Flush all streams as per SUSv2.  */
  if (_GLOBAL_REENT->__cleanup)
    _GLOBAL_REENT->__cleanup (_GLOBAL_REENT);
  do_exit (SIGABRT);	/* signal handler didn't exit.  Goodbye. */
}

static int __reg3
sigaction_worker (int sig, const struct sigaction *newact,
		  struct sigaction *oldact, bool isinternal)
{
  int res = -1;
  __try
    {
      sig_dispatch_pending ();
      /* check that sig is in right range */
      if (sig <= 0 || sig >= NSIG)
	set_errno (EINVAL);
      else
	{
	  struct sigaction oa = global_sigs[sig];

	  if (!newact)
	    sigproc_printf ("signal %d, newact %p, oa %p",
			    sig, newact, oa, oa.sa_handler);
	  else
	    {
	      sigproc_printf ("signal %d, newact %p (handler %p), oa %p",
			      sig, newact, newact->sa_handler, oa,
			      oa.sa_handler);
	      if (sig == SIGKILL || sig == SIGSTOP)
		{
		  set_errno (EINVAL);
		  __leave;
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
	    res = 0;
	}
    }
  __except (EFAULT) {}
  __endtry
  return res;
}

extern "C" int
sigaction (int sig, const struct sigaction *newact, struct sigaction *oldact)
{
  int res = sigaction_worker (sig, newact, oldact, false);
  syscall_printf ("%R = sigaction(%d, %p, %p)", res, sig, newact, oldact);
  return res;
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
  int res = handle_sigsuspend (*set);
  syscall_printf ("%R = sigsuspend(%p)", res, set);
  return res;
}

extern "C" int
sigpause (int signal_mask)
{
  int res = handle_sigsuspend ((sigset_t) signal_mask);
  syscall_printf ("%R = sigpause(%y)", res, signal_mask);
  return res;
}

extern "C" int
pause (void)
{
  int res = handle_sigsuspend (_my_tls.sigmask);
  syscall_printf ("%R = pause()", res);
  return res;
}

extern "C" int
siginterrupt (int sig, int flag)
{
  struct sigaction act;
  int res = sigaction_worker (sig, NULL, &act, false);
  if (res == 0)
    {
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
      res = sigaction_worker (sig, &act, NULL, true);
    }
  syscall_printf ("%R = siginterrupt(%d, %y)", sig, flag);
  return res;
}

extern "C" int
sigwait (const sigset_t *set, int *sig_ptr)
{
  int sig;

  do
    {
      sig = sigwaitinfo (set, NULL);
    }
  while (sig == -1 && get_errno () == EINTR);
  if (sig > 0)
    *sig_ptr = sig;
  return sig > 0 ? 0 : get_errno ();
}

extern "C" int
sigwaitinfo (const sigset_t *set, siginfo_t *info)
{
  int res = -1;

  pthread_testcancel ();

  __try
    {
      set_signal_mask (_my_tls.sigwait_mask, *set);
      sig_dispatch_pending (true);

      switch (cygwait (NULL, cw_infinite, cw_sig_eintr | cw_cancel | cw_cancel_self))
	{
	case WAIT_SIGNALED:
	  if (!sigismember (set, _my_tls.infodata.si_signo))
	    set_errno (EINTR);
	  else
	    {
	      _my_tls.lock ();
	      if (info)
		*info = _my_tls.infodata;
	      res = _my_tls.infodata.si_signo;
	      _my_tls.sig = 0;
	      if (_my_tls.retaddr () == (__tlsstack_t) sigdelayed)
		_my_tls.pop ();
	      _my_tls.unlock ();
	    }
	  break;
	default:
	  __seterrno ();
	  break;
	}
    }
  __except (EFAULT) {
    res = -1;
  }
  __endtry
  sigproc_printf ("returning signal %d", res);
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
  if (sig == 0)
    return 0;
  if (sig < 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      return -1;
    }
  si.si_signo = sig;
  si.si_code = SI_QUEUE;
  si.si_value = value;
  return sig_send (dest, si);
}

extern "C" int
sigaltstack (const stack_t *ss, stack_t *oss)
{
  _cygtls& me = _my_tls;

  __try
    {
      if (ss)
	{
	  if (me.altstack.ss_flags == SS_ONSTACK)
	    {
	      /* An attempt was made to modify an active stack. */
	      set_errno (EPERM);
	      return -1;
	    }
	  if (ss->ss_flags == SS_DISABLE)
	    {
	      me.altstack.ss_sp = NULL;
	      me.altstack.ss_flags = 0;
	      me.altstack.ss_size = 0;
	    }
	  else
	    {
	      if (ss->ss_flags)
		{
		  /* The ss argument is not a null pointer, and the ss_flags
		     member pointed to by ss contains flags other than
		     SS_DISABLE. */
		  set_errno (EINVAL);
		  return -1;
		}
	      if (ss->ss_size < MINSIGSTKSZ)
		{
		  /* The size of the alternate stack area is less than
		     MINSIGSTKSZ. */
		  set_errno (ENOMEM);
		  return -1;
		}
	      memcpy (&me.altstack, ss, sizeof *ss);
	    }
	}
      if (oss)
	{
	  char stack_marker;
	  memcpy (oss, &me.altstack, sizeof *oss);
	  /* Check if the current stack is the alternate signal stack.  If so,
	     set ss_flags accordingly.  We do this here rather than setting
	     ss_flags in _cygtls::call_signal_handler since the signal handler
	     calls longjmp, so we never return to reset the flag. */
	  if (!me.altstack.ss_flags && me.altstack.ss_sp)
	    {
	      if (&stack_marker >= (char *) me.altstack.ss_sp
		  && &stack_marker < (char *) me.altstack.ss_sp
				     + me.altstack.ss_size)
		oss->ss_flags = SS_ONSTACK;
	    }
	}
    }
  __except (EFAULT)
    {
      return EFAULT;
    }
  __endtry
  return 0;
}
