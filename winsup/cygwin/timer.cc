/* timer.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "thread.h"
#include "cygtls.h"
#include "sigproc.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"

#define TT_MAGIC 0x513e4a1c
struct timer_tracker
{
  unsigned magic;
  clockid_t clock_id;
  sigevent evp;
  timespec it_interval;
  HANDLE hcancel;
  HANDLE syncthread;
  long long interval_us;
  long long sleepto_us;
  bool cancel ();
  struct timer_tracker *next;
  int settime (int, const itimerspec *, itimerspec *);
  void gettime (itimerspec *);
  timer_tracker (clockid_t, const sigevent *);
  ~timer_tracker ();
  friend void fixup_timers_after_fork ();
};

timer_tracker NO_COPY ttstart (CLOCK_REALTIME, NULL);

class lock_timer_tracker
{
  static muto protect;
public:
  lock_timer_tracker ();
  ~lock_timer_tracker ();
};

muto NO_COPY lock_timer_tracker::protect;

lock_timer_tracker::lock_timer_tracker ()
{
  protect.init ("timer_protect")->acquire ();
}

lock_timer_tracker::~lock_timer_tracker ()
{
  protect.release ();
}

bool
timer_tracker::cancel ()
{
  if (!hcancel)
    return false;

  SetEvent (hcancel);
  DWORD res = WaitForSingleObject (syncthread, INFINITE);
  if (res != WAIT_OBJECT_0)
    system_printf ("WFSO returned unexpected value %u, %E", res);
  return true;
}

timer_tracker::~timer_tracker ()
{
  if (cancel ())
    {
      CloseHandle (hcancel);
#ifdef DEBUGGING
      hcancel = NULL;
#endif
    }
  if (syncthread)
    CloseHandle (syncthread);
  magic = 0;
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
  magic = TT_MAGIC;
  hcancel = NULL;
  if (this != &ttstart)
    {
      lock_timer_tracker here;
      next = ttstart.next;
      ttstart.next = this;
    }
}

static long long
to_us (const timespec& ts)
{
  long long res = ts.tv_sec;
  res *= 1000000;
  res += ts.tv_nsec / 1000 + ((ts.tv_nsec % 1000) ? 1 : 0);
  return res;
}

static DWORD WINAPI
timer_thread (VOID *x)
{
  timer_tracker *tt = ((timer_tracker *) x);
  long long now;
  long long sleepto_us =  tt->sleepto_us;
  while (1)
    {
      long long sleep_us;
      LONG sleep_ms;
      /* Account for delays in starting thread
	and sending the signal */
      now = gtod.usecs ();
      sleep_us = sleepto_us - now;
      if (sleep_us > 0)
	{
	  tt->sleepto_us = sleepto_us;
	  sleep_ms = (sleep_us + 999) / 1000;
	}
      else
	{
	  tt->sleepto_us = now;
	  sleep_ms = 0;
	}

      debug_printf ("%p waiting for %u ms", x, sleep_ms);
      switch (WaitForSingleObject (tt->hcancel, sleep_ms))
	{
	case WAIT_TIMEOUT:
	  debug_printf ("timed out");
	  break;
	case WAIT_OBJECT_0:
	  debug_printf ("%p cancelled", x);
	  goto out;
	default:
	  debug_printf ("%p wait failed, %E", x);
	  goto out;
	}

      switch (tt->evp.sigev_notify)
	{
	case SIGEV_SIGNAL:
	  {
	    siginfo_t si = {0};
	    si.si_signo = tt->evp.sigev_signo;
	    si.si_sigval.sival_ptr = tt->evp.sigev_value.sival_ptr;
	    si.si_code = SI_TIMER;
	    debug_printf ("%p sending signal %d", x, tt->evp.sigev_signo);
	    sig_send (myself_nowait, si);
	    break;
	  }
	case SIGEV_THREAD:
	  {
	    pthread_t notify_thread;
	    debug_printf ("%p starting thread", x);
	    pthread_attr_t *attr;
	    pthread_attr_t default_attr;
	    if (tt->evp.sigev_notify_attributes)
	      attr = tt->evp.sigev_notify_attributes;
	    else
	      {
		pthread_attr_init(attr = &default_attr);
		pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);
	      }

	    int rc = pthread_create (&notify_thread, attr,
				     (void * (*) (void *)) tt->evp.sigev_notify_function,
				     tt->evp.sigev_value.sival_ptr);
	    if (rc)
	      {
		debug_printf ("thread creation failed, %E");
		return 0;
	      }
	    // FIXME: pthread_join?
	    break;
	  }
	}
      if (!tt->interval_us)
	break;

      sleepto_us = tt->sleepto_us + tt->interval_us;
      debug_printf ("looping");
    }

out:
  _my_tls._ctinfo->auto_release ();     /* automatically return the cygthread to the cygthread pool */
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
  int ret = -1;

  __try
    {
      if (!value)
	{
	  set_errno (EINVAL);
	  __leave;
	}

      if (it_bad (value->it_value) || it_bad (value->it_interval))
	__leave;

      long long now = in_flags & TIMER_ABSTIME ? 0 : gtod.usecs ();

      lock_timer_tracker here;
      cancel ();

      if (ovalue)
	gettime (ovalue);

      if (!value->it_value.tv_sec && !value->it_value.tv_nsec)
	interval_us = sleepto_us = 0;
      else
	{
	  sleepto_us = now + to_us (value->it_value);
	  interval_us = to_us (value->it_interval);
	  it_interval = value->it_interval;
	  if (!hcancel)
	    hcancel = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
	  else
	    ResetEvent (hcancel);
	  if (!syncthread)
	    syncthread = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
	  else
	    ResetEvent (syncthread);
	  new cygthread (timer_thread, this, "itimer", syncthread);
	}
      ret = 0;
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

void
timer_tracker::gettime (itimerspec *ovalue)
{
  if (!hcancel)
    memset (ovalue, 0, sizeof (*ovalue));
  else
    {
      ovalue->it_interval = it_interval;
      long long now = gtod.usecs ();
      long long left_us = sleepto_us - now;
      if (left_us < 0)
       left_us = 0;
      ovalue->it_value.tv_sec = left_us / 1000000;
      ovalue->it_value.tv_nsec = (left_us % 1000000) * 1000;
    }
}

extern "C" int
timer_gettime (timer_t timerid, struct itimerspec *ovalue)
{
  int ret = -1;

  __try
    {
      timer_tracker *tt = (timer_tracker *) timerid;
      if (tt->magic != TT_MAGIC)
	{
	  set_errno (EINVAL);
	  return -1;
	}

      tt->gettime (ovalue);
      ret = 0;
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

extern "C" int
timer_create (clockid_t clock_id, struct sigevent *__restrict evp,
	      timer_t *__restrict timerid)
{
  int ret = -1;

  __try
    {
      if (CLOCKID_IS_PROCESS (clock_id) || CLOCKID_IS_THREAD (clock_id))
	{
	  set_errno (ENOTSUP);
	  return -1;
	}

      if (clock_id != CLOCK_REALTIME)
	{
	  set_errno (EINVAL);
	  return -1;
	}

      *timerid = (timer_t) new timer_tracker (clock_id, evp);
      ret = 0;
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

extern "C" int
timer_settime (timer_t timerid, int flags,
	       const struct itimerspec *__restrict value,
	       struct itimerspec *__restrict ovalue)
{
  int ret = -1;

  __try
    {
      timer_tracker *tt = (timer_tracker *) timerid;
      if (tt->magic != TT_MAGIC)
	{
	  set_errno (EINVAL);
	  __leave;
	}
      ret = tt->settime (flags, value, ovalue);
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

extern "C" int
timer_delete (timer_t timerid)
{
  int ret = -1;

  __try
    {
      timer_tracker *in_tt = (timer_tracker *) timerid;
      if (in_tt->magic != TT_MAGIC)
	{
	  set_errno (EINVAL);
	  __leave;
	}

      lock_timer_tracker here;
      for (timer_tracker *tt = &ttstart; tt->next != NULL; tt = tt->next)
	if (tt->next == in_tt)
	  {
	    tt->next = in_tt->next;
	    delete in_tt;
	    ret = 0;
	    __leave;
	  }
      set_errno (EINVAL);
      ret = 0;
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

void
fixup_timers_after_fork ()
{
  ttstart.hcancel = ttstart.syncthread = NULL;
  for (timer_tracker *tt = &ttstart; tt->next != NULL; /* nothing */)
    {
      timer_tracker *deleteme = tt->next;
      tt->next = deleteme->next;
      deleteme->hcancel = deleteme->syncthread = NULL;
      delete deleteme;
    }
}


extern "C" int
setitimer (int which, const struct itimerval *__restrict value,
	   struct itimerval *__restrict ovalue)
{
  int ret;
  if (which != ITIMER_REAL)
    {
      set_errno (EINVAL);
      ret = -1;
    }
  else
    {
      struct itimerspec spec_value, spec_ovalue;
      spec_value.it_interval.tv_sec = value->it_interval.tv_sec;
      spec_value.it_interval.tv_nsec = value->it_interval.tv_usec * 1000;
      spec_value.it_value.tv_sec = value->it_value.tv_sec;
      spec_value.it_value.tv_nsec = value->it_value.tv_usec * 1000;
      ret = timer_settime ((timer_t) &ttstart, 0, &spec_value, &spec_ovalue);
      if (ret)
	ret = -1;
      else if (ovalue)
	{
	  ovalue->it_interval.tv_sec = spec_ovalue.it_interval.tv_sec;
	  ovalue->it_interval.tv_usec = spec_ovalue.it_interval.tv_nsec / 1000;
	  ovalue->it_value.tv_sec = spec_ovalue.it_value.tv_sec;
	  ovalue->it_value.tv_usec = spec_ovalue.it_value.tv_nsec / 1000;
	}
    }
  syscall_printf ("%R = setitimer()", ret);
  return ret;
}


extern "C" int
getitimer (int which, struct itimerval *ovalue)
{
  int ret = -1;

  if (which != ITIMER_REAL)
    set_errno (EINVAL);
  else
    {
      __try
	{
	  struct itimerspec spec_ovalue;
	  ret = timer_gettime ((timer_t) &ttstart, &spec_ovalue);
	  if (!ret)
	    {
	      ovalue->it_interval.tv_sec = spec_ovalue.it_interval.tv_sec;
	      ovalue->it_interval.tv_usec = spec_ovalue.it_interval.tv_nsec / 1000;
	      ovalue->it_value.tv_sec = spec_ovalue.it_value.tv_sec;
	      ovalue->it_value.tv_usec = spec_ovalue.it_value.tv_nsec / 1000;
	    }
	}
      __except (EFAULT) {}
      __endtry
    }
  syscall_printf ("%R = getitimer()", ret);
  return ret;
}

/* FIXME: POSIX - alarm survives exec */
extern "C" unsigned int
alarm (unsigned int seconds)
{
 struct itimerspec newt = {}, oldt;
 /* alarm cannot fail, but only needs not be
    correct for arguments < 64k. Truncate */
 if (seconds > (HIRES_DELAY_MAX / 1000 - 1))
   seconds = (HIRES_DELAY_MAX / 1000 - 1);
 newt.it_value.tv_sec = seconds;
 timer_settime ((timer_t) &ttstart, 0, &newt, &oldt);
 int ret = oldt.it_value.tv_sec + (oldt.it_value.tv_nsec > 0);
 syscall_printf ("%d = alarm(%u)", ret, seconds);
 return ret;
}

extern "C" useconds_t
ualarm (useconds_t value, useconds_t interval)
{
 struct itimerspec timer = {}, otimer;
 /* ualarm cannot fail.
    Interpret negative arguments as zero */
 if (value > 0)
   {
     timer.it_value.tv_sec = value / 1000000;
     timer.it_value.tv_nsec = (value % 1000000) * 1000;
   }
 if (interval > 0)
   {
     timer.it_interval.tv_sec = interval / 1000000;
     timer.it_interval.tv_nsec = (interval % 1000000) * 1000;
   }
 timer_settime ((timer_t) &ttstart, 0, &timer, &otimer);
 useconds_t ret = otimer.it_value.tv_sec * 1000000 + (otimer.it_value.tv_nsec + 999) / 1000;
 syscall_printf ("%d = ualarm(%ld , %ld)", ret, value, interval);
 return ret;
}
