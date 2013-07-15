/* cygwait.h

   Copyright 2011, 2012, 2013 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details.  */

#include "winsup.h"
#include "sigproc.h"
#include "cygwait.h"
#include "ntdll.h"

#define is_cw_cancel		(mask & cw_cancel)
#define is_cw_cancel_self	(mask & cw_cancel_self)
#define is_cw_sig		(mask & cw_sig)
#define is_cw_sig_eintr		(mask & cw_sig_eintr)
#define is_cw_sig_cont		(mask & cw_sig_cont)

#define is_cw_sig_handle	(mask & (cw_sig | cw_sig_eintr | cw_sig_cont))

LARGE_INTEGER cw_nowait_storage;

DWORD
cygwait (HANDLE object, PLARGE_INTEGER timeout, unsigned mask)
{
  DWORD res;
  DWORD num = 0;
  HANDLE wait_objects[4];
  pthread_t thread = pthread::self ();

  /* Do not change the wait order.
     The object must have higher priority than the cancel event,
     because WaitForMultipleObjects will return the smallest index
     if both objects are signaled. */
  if (object)
    wait_objects[num++] = object;

  set_signal_arrived thread_waiting (is_cw_sig_handle, wait_objects[num]);
  debug_only_printf ("object %p, thread waiting %d, signal_arrived %p", object, (int) thread_waiting, _my_tls.signal_arrived);
  DWORD sig_n;
  if (!thread_waiting)
    sig_n = WAIT_TIMEOUT + 1;
  else
    sig_n = WAIT_OBJECT_0 + num++;

  DWORD cancel_n;
  if (!is_cw_cancel || !pthread::is_good_object (&thread) ||
      thread->cancelstate == PTHREAD_CANCEL_DISABLE)
    cancel_n = WAIT_TIMEOUT + 1;
  else
    {
      cancel_n = WAIT_OBJECT_0 + num++;
      wait_objects[cancel_n] = thread->cancel_event;
    }

  DWORD timeout_n;
  if (!timeout)
    timeout_n = WAIT_TIMEOUT + 1;
  else
    {
      timeout_n = WAIT_OBJECT_0 + num++;
      if (!_my_tls.locals.cw_timer)
	NtCreateTimer (&_my_tls.locals.cw_timer, TIMER_ALL_ACCESS, NULL,
		       NotificationTimer);
      NtSetTimer (_my_tls.locals.cw_timer, timeout, NULL, NULL, FALSE, 0, NULL);
      wait_objects[timeout_n] = _my_tls.locals.cw_timer;
    }

  while (1)
    {
      res = WaitForMultipleObjects (num, wait_objects, FALSE, INFINITE);
      debug_only_printf ("res %d", res);
      if (res == cancel_n)
	res = WAIT_CANCELED;
      else if (res == timeout_n)
	res = WAIT_TIMEOUT;
      else if (res != sig_n)
	/* all set */;
      else
	{
	  int sig = _my_tls.sig;
	  if (is_cw_sig_cont && sig == SIGCONT)
	    _my_tls.sig = 0;
	  if (!sig)
	    continue;
	  if (is_cw_sig_eintr || (is_cw_sig_cont && sig == SIGCONT))
	    res = WAIT_SIGNALED;	/* caller will deal with signals */
	  else if (_my_tls.call_signal_handler ())
	    continue;
	}
      break;
    }

  if (timeout)
    {
      TIMER_BASIC_INFORMATION tbi;

      NtQueryTimer (_my_tls.locals.cw_timer, TimerBasicInformation, &tbi,
		    sizeof tbi, NULL);
      /* if timer expired, TimeRemaining is negative and represents the
	  system uptime when signalled */
      if (timeout->QuadPart < 0LL)
	timeout->QuadPart = tbi.SignalState ? 0LL : tbi.TimeRemaining.QuadPart;
      NtCancelTimer (_my_tls.locals.cw_timer, NULL);
    }

  if (res == WAIT_CANCELED && is_cw_cancel_self)
    pthread::static_cancel_self ();

  return res;
}
