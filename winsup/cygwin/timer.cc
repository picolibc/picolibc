/* timer.cc

   Copyright 2004 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <time.h>
#include <stdlib.h>
#include "cygerrno.h"
#include "security.h"
#include "hires.h"
#include "thread.h"
#include "cygtls.h"
#include "cygthread.h"
#include "sigproc.h"
#include "sync.h"

#define TT_MAGIC 0x513e4a1c
struct timer_tracker
{
  static muto *protect;
  unsigned magic;
  clockid_t clock_id;
  sigevent evp;
  itimerspec it;
  HANDLE cancel;
  int flags;
  cygthread *th;
  struct timer_tracker *next;
  int settime (int, const itimerspec *, itimerspec *);
  timer_tracker (clockid_t, const sigevent *);
  timer_tracker ();
};

timer_tracker ttstart;

muto *timer_tracker::protect;

timer_tracker::timer_tracker ()
{
  new_muto (protect);
}

timer_tracker::timer_tracker (clockid_t c, const sigevent *e)
{
  if (e != NULL)
    evp = *e;
  else
    {
      evp.sigev_notify = SIGEV_SIGNAL;
      evp.sigev_signo = SIGALRM;
      evp.sigev_value.sival_ptr = this;
    }
  clock_id = c;
  cancel = NULL;
  flags = 0;
  memset (&it, 0, sizeof (it));
  protect->acquire ();
  next = ttstart.next;
  ttstart.next = this;
  protect->release ();
  magic = TT_MAGIC;
}

static long long
to_us (timespec& ts)
{
  long long res = ts.tv_sec;
  res *= 1000000;
  res += ts.tv_nsec / 1000 + ((ts.tv_nsec % 1000) >= 500 ? 1 : 0);
  return res;
}

static NO_COPY itimerspec itzero;
static NO_COPY timespec tzero;

static DWORD WINAPI
timer_thread (VOID *x)
{
  timer_tracker *tp = ((timer_tracker *) x);
  timer_tracker tt = *tp;
  for (bool first = true; ; first = false)
    {
      long long sleep_us = to_us (first ? tt.it.it_value : tt.it.it_interval);
      long long sleep_to = sleep_us;
      long long now = gtod.usecs (false);
      if (tt.flags & TIMER_ABSTIME)
	sleep_us -= now;
      else
	sleep_to += now;

      DWORD sleep_ms = (sleep_us < 0) ? 0 : (sleep_us / 1000);
      debug_printf ("%p waiting for %u ms, first %d", x, sleep_ms, first);
      tp->it.it_value = tzero;
      switch (WaitForSingleObject (tt.cancel, sleep_ms))
	{
	case WAIT_TIMEOUT:
	  debug_printf ("timed out");
	  break;
	case WAIT_OBJECT_0:
	  now = gtod.usecs (false);
	  sleep_us = sleep_to - now;
	  if (sleep_us < 0)
	    sleep_us = 0;
	  tp->it.it_value.tv_sec = sleep_us / 1000000;
	  tp->it.it_value.tv_nsec = (sleep_us % 1000000) * 1000;
	  debug_printf ("%p cancelled, elapsed %D", x, sleep_us);
	  goto out;
	default:
	  debug_printf ("%p timer wait failed, %E", x);
	  goto out;
	}

      switch (tt.evp.sigev_notify)
	{
	case SIGEV_SIGNAL:
	  {
	    siginfo_t si;
	    memset (&si, 0, sizeof (si));
	    si.si_signo = tt.evp.sigev_signo;
	    si.si_sigval.sival_ptr = tt.evp.sigev_value.sival_ptr;
	    debug_printf ("%p sending sig %d", x, tt.evp.sigev_signo);
	    sig_send (NULL, si);
	    break;
	  }
	case SIGEV_THREAD:
	  {
	    pthread_t notify_thread;
	    debug_printf ("%p starting thread", x);
	    int rc = pthread_create (&notify_thread, tt.evp.sigev_notify_attributes,
				     (void * (*) (void *)) tt.evp.sigev_notify_function,
				     &tt.evp.sigev_value);
	    if (rc)
	      {
		debug_printf ("thread creation failed, %E");
		return 0;
	      }
	    // FIXME: pthread_join?
	    break;
	  }
	}
      if (!tt.it.it_interval.tv_sec && !tt.it.it_interval.tv_nsec)
	break;
      tt.flags = 0;
      debug_printf ("looping");
    }

out:
  CloseHandle (tt.cancel);
  // FIXME: race here but is it inevitable?
  if (tt.cancel == tp->cancel)
    tp->cancel = NULL;
  return 0;
}

static bool
it_bad (const timespec& t)
{
  if (t.tv_nsec < 0 || t.tv_nsec >= 1000000000 || t.tv_sec < 0)
    {
      set_errno (EINVAL);
      return true;
    }
  return false;
}

int
timer_tracker::settime (int in_flags, const itimerspec *value, itimerspec *ovalue)
{
  if (!value)
    {
      set_errno (EINVAL);
      return -1;
    }

  if (__check_invalid_read_ptr_errno (value, sizeof (*value)))
    return -1;

  if (ovalue && check_null_invalid_struct_errno (ovalue))
    return -1;

  itimerspec *elapsed;
  if (!cancel)
    elapsed = &itzero;
  else
    {
      SetEvent (cancel);	// should be closed when the thread exits
      th->detach ();
      elapsed = &it;
    }

  if (ovalue)
    *ovalue = *elapsed;

  if (value->it_value.tv_sec || value->it_value.tv_nsec)
    {
      if (it_bad (value->it_value))
	return -1;
      if (it_bad (value->it_interval))
	return -1;
      flags = in_flags;
      cancel = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
      it = *value;
      th = new cygthread (timer_thread, this, "itimer");
    }

  return 0;
}

extern "C" int
timer_create (clockid_t clock_id, struct sigevent *evp, timer_t *timerid)
{
  if (evp && check_null_invalid_struct_errno (evp))
    return -1;
  if (check_null_invalid_struct_errno (timerid))
    return -1;

  if (clock_id != CLOCK_REALTIME)
    {
      set_errno (EINVAL);
      return -1;
    }

  *timerid = (timer_t) new timer_tracker (clock_id, evp);
  return 0;
}

extern "C" int
timer_settime (timer_t timerid, int flags, const struct itimerspec *value,
	       struct itimerspec *ovalue)
{
  timer_tracker *tt = (timer_tracker *) timerid;
  if (check_null_invalid_struct_errno (tt) || tt->magic != TT_MAGIC)
    return -1;
  return tt->settime (flags, value, ovalue);
}

extern "C" int
timer_delete (timer_t timerid)
{
  timer_tracker *in_tt = (timer_tracker *) timerid;
  if (check_null_invalid_struct_errno (in_tt) || in_tt->magic != TT_MAGIC)
    return -1;

  timer_tracker::protect->acquire ();
  for (timer_tracker *tt = &ttstart; tt->next != NULL; tt = tt->next)
    if (tt->next == in_tt)
      {
	timer_tracker *deleteme = tt->next;
	tt->next = deleteme->next;
	delete deleteme;
	timer_tracker::protect->release ();
	return 0;
      }
  timer_tracker::protect->release ();

  set_errno (EINVAL);
  return 0;
}

void
fixup_timers_after_fork ()
{
  for (timer_tracker *tt = &ttstart; tt->next != NULL; /* nothing */)
    {
      timer_tracker *deleteme = tt->next;
      tt->next = deleteme->next;
      delete deleteme;
    }
}
