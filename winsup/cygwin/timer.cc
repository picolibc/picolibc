/* timer.cc: posix timers

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
#include "timer.h"
#include <sys/timerfd.h>
#include <sys/param.h>

#define EVENT_DISARMED	 0
#define EVENT_ARMED	-1
#define EVENT_LOCK	 1

timer_tracker NO_COPY ttstart (CLOCK_REALTIME, NULL, false);

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
  HANDLE hdl;

  deleting = true;
  if (cancel ())
    {
      HANDLE hdl = InterlockedExchangePointer (&hcancel, NULL);
      CloseHandle (hdl);
      hdl = InterlockedExchangePointer (&timerfd_event, NULL);
      if (hdl)
	CloseHandle (hdl);
    }
  hdl = InterlockedExchangePointer (&syncthread, NULL);
  if (hdl)
    CloseHandle (hdl);
  magic = 0;
}

/* fd is true for timerfd timers. */
timer_tracker::timer_tracker (clockid_t c, const sigevent *e, bool fd)
: magic (TT_MAGIC), instance_count (1), clock_id (c), deleting (false),
  hcancel (NULL), syncthread (NULL), event_running (EVENT_DISARMED),
  overrun_count_curr (0), overrun_count (0)
{
  if (e != NULL)
    evp = *e;
  else if (fd)
    {
      evp.sigev_notify = SIGEV_NONE;
      evp.sigev_signo = 0;
      evp.sigev_value.sival_ptr = this;
    }
  else
    {
      evp.sigev_notify = SIGEV_SIGNAL;
      evp.sigev_signo = SIGALRM;
      evp.sigev_value.sival_ptr = this;
    }
  if (fd)
    timerfd_event = CreateEvent (&sec_none_nih, TRUE, FALSE, NULL);
  if (this != &ttstart)
    {
      lock_timer_tracker here;
      next = ttstart.next;
      ttstart.next = this;
    }
}

void timer_tracker::increment_instances ()
{
  InterlockedIncrement (&instance_count);
}

LONG timer_tracker::decrement_instances ()
{
  return InterlockedDecrement (&instance_count);
}

static inline int64_t
timespec_to_us (const timespec& ts)
{
  int64_t res = ts.tv_sec;
  res *= USPERSEC;
  res += (ts.tv_nsec + (NSPERSEC/USPERSEC) - 1) / (NSPERSEC/USPERSEC);
  return res;
}

/* Returns 0 if arming successful, -1 if a signal is already queued.
   If so, it also increments overrun_count. */
LONG
timer_tracker::arm_event ()
{
  LONG ret;

  while ((ret = InterlockedCompareExchange (&event_running, EVENT_ARMED,
					    EVENT_DISARMED)) == EVENT_LOCK)
    yield ();
  if (ret == EVENT_ARMED)
    InterlockedIncrement64 (&overrun_count);
  return ret;
}

LONG64
timer_tracker::_disarm_event ()
{
  LONG ret;

  while ((ret = InterlockedCompareExchange (&event_running, EVENT_LOCK,
					    EVENT_ARMED)) == EVENT_LOCK)
    yield ();
  if (ret == EVENT_ARMED)
    {

      InterlockedExchange64 (&overrun_count_curr, overrun_count);
      ret = overrun_count_curr;
      InterlockedExchange64 (&overrun_count, 0);
      InterlockedExchange (&event_running, EVENT_DISARMED);
    }
  return ret;
}

unsigned int
timer_tracker::disarm_event ()
{
  LONG64 ov = _disarm_event ();
  if (ov > DELAYTIMER_MAX || ov < 0)
    return DELAYTIMER_MAX;
  return (unsigned int) ov;
}

LONG64
timer_tracker::wait (bool nonblocking)
{
  HANDLE w4[3] = { NULL, hcancel, timerfd_event };
  LONG64 ret = -1;

  wait_signal_arrived here (w4[0]);
repeat:
  switch (WaitForMultipleObjects (3, w4, FALSE, nonblocking ? 0 : INFINITE))
    {
    case WAIT_OBJECT_0:		/* signal */
      if (_my_tls.call_signal_handler ())
	goto repeat;
      set_errno (EINTR);
      break;
    case WAIT_OBJECT_0 + 1:	/* settime oder timer delete */
      if (deleting)
	{
	  set_errno (EIO);
	  break;
	}
      /*FALLTHRU*/
    case WAIT_OBJECT_0 + 2:	/* timer event */
      ret = _disarm_event ();
      ResetEvent (timerfd_event);
      break;
    case WAIT_TIMEOUT:
      set_errno (EAGAIN);
      break;
    default:
      __seterrno ();
      break;
    }
  return ret;
}

static void *
notify_thread_wrapper (void *arg)
{
  timer_tracker *tt = (timer_tracker *) arg;
  sigevent_t *evt = tt->sigevt ();
  void * (*notify_func) (void *) = (void * (*) (void *))
				   evt->sigev_notify_function;

  tt->disarm_event ();
  return notify_func (evt->sigev_value.sival_ptr);
}

DWORD
timer_tracker::thread_func ()
{
  int64_t now;
  int64_t cur_sleepto_us = sleepto_us;
  while (1)
    {
      int64_t sleep_us;
      LONG sleep_ms;
      /* Account for delays in starting thread
	and sending the signal */
      now = get_clock (clock_id)->usecs ();
      sleep_us = cur_sleepto_us - now;
      if (sleep_us > 0)
	{
	  sleepto_us = cur_sleepto_us;
	  sleep_ms = (sleep_us + (USPERSEC / MSPERSEC) - 1)
		     / (USPERSEC / MSPERSEC);
	}
      else
	{
	  int64_t num_intervals = (now - cur_sleepto_us) / interval_us;
	  InterlockedAdd64 (&overrun_count, num_intervals);
	  cur_sleepto_us += num_intervals * interval_us;
	  sleepto_us = cur_sleepto_us;
	  sleep_ms = 0;
	}

      debug_printf ("%p waiting for %u ms", this, sleep_ms);
      switch (WaitForSingleObject (hcancel, sleep_ms))
	{
	case WAIT_TIMEOUT:
	  debug_printf ("timed out");
	  break;
	case WAIT_OBJECT_0:
	  debug_printf ("%p cancelled", this);
	  goto out;
	default:
	  debug_printf ("%p wait failed, %E", this);
	  goto out;
	}

      switch (evp.sigev_notify)
	{
	case SIGEV_NONE:
	  {
	    if (!timerfd_event)
	      break;
	    arm_event ();
	    SetEvent (timerfd_event);
	    break;
	  }
	case SIGEV_SIGNAL:
	  {
	    if (arm_event ())
	      {
		debug_printf ("%p timer signal already queued", this);
		break;
	      }
	    siginfo_t si = {0};
	    si.si_signo = evp.sigev_signo;
	    si.si_code = SI_TIMER;
	    si.si_tid = (timer_t) this;
	    si.si_sigval.sival_ptr = evp.sigev_value.sival_ptr;
	    debug_printf ("%p sending signal %d", this, evp.sigev_signo);
	    sig_send (myself_nowait, si);
	    break;
	  }
	case SIGEV_THREAD:
	  {
	    if (arm_event ())
	      {
		debug_printf ("%p timer thread already queued", this);
		break;
	      }
	    pthread_t notify_thread;
	    debug_printf ("%p starting thread", this);
	    pthread_attr_t *attr;
	    pthread_attr_t default_attr;
	    if (evp.sigev_notify_attributes)
	      attr = evp.sigev_notify_attributes;
	    else
	      {
		pthread_attr_init(attr = &default_attr);
		pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);
	      }
	    int rc = pthread_create (&notify_thread, attr,
				     notify_thread_wrapper, this);
	    if (rc)
	      {
		debug_printf ("thread creation failed, %E");
		return 0;
	      }
	    break;
	  }
	}
      if (!interval_us)
	break;

      cur_sleepto_us = sleepto_us + interval_us;
      debug_printf ("looping");
    }

out:
  _my_tls._ctinfo->auto_release ();     /* automatically return the cygthread to the cygthread pool */
  return 0;
}

static DWORD WINAPI
timer_thread (VOID *x)
{
  timer_tracker *tt = ((timer_tracker *) x);
  return tt->thread_func ();
}

static inline bool
timespec_bad (const timespec& t)
{
  if (t.tv_nsec < 0 || t.tv_nsec >= NSPERSEC || t.tv_sec < 0)
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

      if (timespec_bad (value->it_value) || timespec_bad (value->it_interval))
	__leave;

      lock_timer_tracker here;
      cancel ();

      if (ovalue)
	gettime (ovalue);

      if (!value->it_value.tv_sec && !value->it_value.tv_nsec)
	interval_us = sleepto_us = 0;
      else
	{
	  interval_us = timespec_to_us (value->it_interval);
	  sleepto_us = timespec_to_us (value->it_value);
	  if (!(in_flags & TIMER_ABSTIME))
	    sleepto_us += get_clock (clock_id)->usecs ();
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
      int64_t now = get_clock (clock_id)->usecs ();
      int64_t left_us = sleepto_us - now;
      if (left_us < 0)
       left_us = 0;
      ovalue->it_value.tv_sec = left_us / USPERSEC;
      ovalue->it_value.tv_nsec = (left_us % USPERSEC) * (NSPERSEC/USPERSEC);
    }
}

/* Returns

    1 if we still have to keep the timer around
    0 if we can delete the timer
   -1 if we can't find the timer in the list
*/
int
timer_tracker::clean_and_unhook ()
{
  if (decrement_instances () > 0)
    return 1;
  for (timer_tracker *tt = &ttstart; tt->next != NULL; tt = tt->next)
    if (tt->next == this)
      {
	tt->next = this->next;
	return 0;
      }
  return -1;
}

int
timer_tracker::close (timer_tracker *tt)
{
  lock_timer_tracker here;
  int ret = tt->clean_and_unhook ();
  if (ret >= 0)
    {
      if (ret == 0)
	delete tt;
      ret = 0;
    }
  else
    set_errno (EINVAL);
  return ret;
}

void
timer_tracker::fixup_after_fork ()
{
  /* TODO: Keep timerfd timers available and restart them */
  ttstart.hcancel = ttstart.syncthread = NULL;
  ttstart.event_running = EVENT_DISARMED;
  ttstart.overrun_count_curr = 0;
  ttstart.overrun_count = 0;
  for (timer_tracker *tt = &ttstart; tt->next != NULL; /* nothing */)
    {
      timer_tracker *deleteme = tt->next;
      tt->next = deleteme->next;
      deleteme->hcancel = deleteme->syncthread = NULL;
      delete deleteme;
    }
}

void
fixup_timers_after_fork ()
{
  timer_tracker::fixup_after_fork ();
}

extern "C" int
timer_gettime (timer_t timerid, struct itimerspec *ovalue)
{
  int ret = -1;

  __try
    {
      timer_tracker *tt = (timer_tracker *) timerid;
      if (!tt->is_timer_tracker ())
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

  if (CLOCKID_IS_PROCESS (clock_id) || CLOCKID_IS_THREAD (clock_id))
    {
      set_errno (ENOTSUP);
      return -1;
    }

  if (clock_id >= MAX_CLOCKS)
    {
      set_errno (EINVAL);
      return -1;
    }

  __try
    {
      *timerid = (timer_t) new timer_tracker (clock_id, evp, false);
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
      if (!tt->is_timer_tracker ())
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
timer_getoverrun (timer_t timerid)
{
  int ret = -1;

  __try
    {
      timer_tracker *tt = (timer_tracker *) timerid;
      if (!tt->is_timer_tracker ())
	{
	  set_errno (EINVAL);
	  __leave;
	}
      LONG64 ov_cnt = tt->getoverrun ();
      if (ov_cnt > DELAYTIMER_MAX || ov_cnt < 0)
	ret = DELAYTIMER_MAX;
      else
	ret = ov_cnt;
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
      if (!in_tt->is_timer_tracker ())
	{
	  set_errno (EINVAL);
	  __leave;
	}
      ret = timer_tracker::close (in_tt);
    }
  __except (EFAULT) {}
  __endtry
  return ret;
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
 if (seconds > (CLOCK_DELAY_MAX / 1000 - 1))
   seconds = (CLOCK_DELAY_MAX / 1000 - 1);
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
     timer.it_value.tv_sec = value / USPERSEC;
     timer.it_value.tv_nsec = (value % USPERSEC) * (NSPERSEC/USPERSEC);
   }
 if (interval > 0)
   {
     timer.it_interval.tv_sec = interval / USPERSEC;
     timer.it_interval.tv_nsec = (interval % USPERSEC) * (NSPERSEC/USPERSEC);
   }
 timer_settime ((timer_t) &ttstart, 0, &timer, &otimer);
 useconds_t ret = otimer.it_value.tv_sec * USPERSEC
		  + (otimer.it_value.tv_nsec + (NSPERSEC/USPERSEC) - 1)
		    / (NSPERSEC/USPERSEC);
 syscall_printf ("%d = ualarm(%ld , %ld)", ret, value, interval);
 return ret;
}

extern "C" int
timerfd_create (clockid_t clock_id, int flags)
{
  int ret = -1;
  fhandler_timerfd *fh;

  debug_printf ("timerfd (%lu, %y)", clock_id, flags);

  if (clock_id != CLOCK_REALTIME
      && clock_id != CLOCK_MONOTONIC
      && clock_id != CLOCK_BOOTTIME)
    {
      set_errno (EINVAL);
      return -1;
    }
  if ((flags & ~(TFD_NONBLOCK | TFD_CLOEXEC)) != 0)
    {
      set_errno (EINVAL);
      goto done;
    }

    {
      /* Create new timerfd descriptor. */
      cygheap_fdnew fd;

      if (fd < 0)
        goto done;
      fh = (fhandler_timerfd *) build_fh_dev (*timerfd_dev);
      if (fh && fh->timerfd (clock_id, flags) == 0)
        {
          fd = fh;
          if (fd <= 2)
            set_std_handle (fd);
          ret = fd;
        }
      else
        delete fh;
    }

done:
  syscall_printf ("%R = timerfd (%lu, %y)", ret, clock_id, flags);
  return ret;
}

extern "C" int
timerfd_settime (int fd_in, int flags, const struct itimerspec *value,
		 struct itimerspec *ovalue)
{
  if ((flags & ~(TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET)) != 0)
    {
      set_errno (EINVAL);
      return -1;
    }

  cygheap_fdget fd (fd_in);
  if (fd < 0)
    return -1;
  fhandler_timerfd *fh = fd->is_timerfd ();
  if (!fh)
    {
      set_errno (EINVAL);
      return -1;
    }
  return fh->settime (flags, value, ovalue);
}

extern "C" int
timerfd_gettime (int fd_in, struct itimerspec *ovalue)
{
  cygheap_fdget fd (fd_in);
  if (fd < 0)
    return -1;
  fhandler_timerfd *fh = fd->is_timerfd ();
  if (!fh)
    {
      set_errno (EINVAL);
      return -1;
    }
  return fh->gettime (ovalue);
}
