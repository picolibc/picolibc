/* signal.cc

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

   Written by Steve Chamberlain of Cygnus Support, sac@cygnus.com
   Significant changes by Sergey Okhapkin <sos@prospect.com.ru>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <errno.h>
#include "winsup.h"

extern "C"
_sig_func_ptr
signal (int sig, _sig_func_ptr func)
{
  _sig_func_ptr prev;

  /* check that sig is in right range */
  if (sig < 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("SIG_ERR = signal (%d, %p)", sig, func);
      return (_sig_func_ptr) SIG_ERR;
    }

  prev = myself->getsig(sig).sa_handler;
  myself->getsig(sig).sa_handler = func;
  myself->getsig(sig).sa_mask = 0;
  syscall_printf ("%p = signal (%d, %p)", prev, sig, func);
  return prev;
}

extern "C"
unsigned int
sleep (unsigned int seconds)
{
  int res;
  unsigned start_time;

  start_time = GetTickCount ();

  syscall_printf ("sleep (%d)", seconds);
  res = WaitForSingleObject (signal_arrived, seconds * 1000);
  if (res == WAIT_TIMEOUT)
    {
      syscall_printf ("0 = sleep (%d)", seconds);
      return 0;
    }
  return (GetTickCount () - start_time)/1000;
}

extern "C"
unsigned int
usleep (unsigned int useconds)
{
  syscall_printf ("usleep (%d)", useconds);
  WaitForSingleObject (signal_arrived, (useconds + 500) / 1000);
  syscall_printf ("0 = usleep (%d)", useconds);
  return 0;
}

extern "C"
int
sigprocmask (int sig, const sigset_t *set, sigset_t *oldset)
{
  /* check that sig is in right range */
  if (sig < 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("SIG_ERR = sigprocmask sig %d out of range", sig);
      return -1;
    }

  if (oldset)
    *oldset = myself->getsigmask ();
  if (set)
    {
      sigset_t newmask = myself->getsigmask ();
      switch (sig)
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
	default:
	  set_errno (EINVAL);
	  return -1;
	}
      (void) set_process_mask (newmask);
    }
  return 0;
}

static int
kill_worker (pid_t pid, int sig)
{
  int res = 0;
  pinfo  *dest = procinfo (pid);
  BOOL sendSIGCONT;

  if (!dest)
    {
      set_errno (ESRCH);
      return -1;
    }

  dest->setthread2signal(NULL);

  if ((sendSIGCONT = (sig < 0)))
    sig = -sig;

#if 0
  if (dest == myself && !sendSIGCONT)
    dest = myself_nowait_nonmain;
#endif
  if (sig == 0)
    res = proc_exists (dest) ? 0 : -1;
  else if ((res = sig_send (dest, sig)))
    {
      sigproc_printf ("%d = sig_send, %E ", res);
      res = -1;
    }
  else if (sendSIGCONT)
    (void) sig_send (dest, SIGCONT);

  syscall_printf ("%d = kill_worker (%d, %d)", res, pid, sig);
  return res;
}

/* This is called _kill because the real kill is in newlib.  */
int
_kill (pid_t pid, int sig)
{
  syscall_printf ("kill (%d, %d)", pid, sig);
  /* check that sig is in right range */
  if (sig < 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("sig %d out of range", sig);
      return -1;
    }

  /* Silently ignore stop signals from a member of orphaned process group.
     FIXME: Why??? */
  if (ISSTATE(myself, PID_ORPHANED) &&
      (sig == SIGTSTP || sig == SIGTTIN || sig == SIGTTOU))
    sig = 0;

  return (pid > 0) ? kill_worker (pid, sig) : kill_pgrp (-pid, sig);
}

int
kill_pgrp (pid_t pid, int sig)
{
  int res = 0;
  int found = 0;
  int killself = 0;

  sigproc_printf ("pid %d, sig %d", pid, sig);

  for (int i = 0; i < cygwin_shared->p.size (); i++)
    {
      pinfo *p = &cygwin_shared->p.vec[i];

      if (!proc_exists (p))
	continue;

      /* Is it a process we want to kill?  */
      if (pid == 0 && (p->pgid != myself->pgid || p->ctty != myself->ctty))
	continue;
      if (pid > 1 && p->pgid != pid)
	continue;
      if (sig < 0 && NOTSTATE(p, PID_STOPPED))
	continue;
      sigproc_printf ("killing pid %d, pgrp %d, p->ctty %d, myself->ctty %d",
		      p->pid, p->pgid, p->ctty, myself->ctty);
      if (p == myself)
	killself++;
      else if (kill_worker (p->pid, sig))
	res = -1;
      found++;
    }

  if (killself && kill_worker (myself->pid, sig))
    res = -1;

  if (!found)
    {
      set_errno (ESRCH);
      res = -1;
    }
  syscall_printf ("%d = kill (%d, %d)", res, pid, sig);
  return res;
}

extern "C"
int
killpg (int pgrp, int sig)
{
  return _kill (-pgrp, sig);
}

extern "C"
int
sigaction (int sig,
		const struct sigaction *newaction,
		struct sigaction *oldaction)
{
  /* check that sig is in right range */
  if (sig < 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("SIG_ERR = sigaction sig %d out of range", sig);
      return -1;
    }

  if (oldaction)
      *oldaction = myself->getsig(sig);

  if (newaction)
    {
      if ((sig == SIGKILL || sig == SIGSTOP) && newaction->sa_handler != SIG_DFL)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      myself->getsig(sig) = *newaction;
      if (newaction->sa_handler == SIG_IGN)
	sig_clear (sig);
      if (newaction->sa_handler == SIG_DFL && sig == SIGCHLD)
	sig_clear (sig);
    }

  return 0;
}

extern "C"
int
sigaddset (sigset_t *set, const int sig)
{
  /* check that sig is in right range */
  if (sig <= 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("SIG_ERR = sigaddset sig %d out of range", sig);
      return -1;
    }

  *set |= SIGTOMASK (sig);
  return 0;
}

extern "C"
int
sigdelset (sigset_t *set, const int sig)
{
  /* check that sig is in right range */
  if (sig <= 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("SIG_ERR = sigdelset sig %d out of range", sig);
      return -1;
    }

  *set &= ~SIGTOMASK (sig);
  return 0;
}

extern "C"
int
sigismember (const sigset_t *set, int sig)
{
  /* check that sig is in right range */
  if (sig <= 0 || sig >= NSIG)
    {
      set_errno (EINVAL);
      syscall_printf ("SIG_ERR = sigdelset sig %d out of range", sig);
      return -1;
    }

  if (*set & SIGTOMASK (sig))
    return 1;
  else
    return 0;
}

extern "C"
int
sigemptyset (sigset_t *set)
{
  *set = (sigset_t) 0;
  return 0;
}

extern "C"
int
sigfillset (sigset_t *set)
{
  *set = ~((sigset_t) 0);
  return 0;
}

extern "C"
int
sigpending (sigset_t *set)
{
  unsigned bit;
  *set = 0;
  for (int sig = 1; sig < NSIG; sig++)
    if (*myself->getsigtodo(sig) && myself->getsigmask () & (bit = SIGTOMASK (sig)))
      *set |= bit;
  return 0;
}

extern "C"
int
sigsuspend (const sigset_t *set)
{
  return handle_sigsuspend (*set);
}

extern "C"
int
sigpause (int signal_mask)
{
  return handle_sigsuspend ((sigset_t) signal_mask);
}

extern "C"
int
pause (void)
{
  return handle_sigsuspend (myself->getsigmask ());
}
