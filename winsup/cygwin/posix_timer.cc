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
#include "posix_timer.h"
#include <sys/param.h>

#define OVR_DISARMED		-1LL

timer_tracker NO_COPY itimer_tracker (CLOCK_REALTIME, NULL);

bool
timer_tracker::cancel ()
{
  DWORD res;

  if (!cancel_evt)
    return false;
  SetEvent (cancel_evt);
  if (sync_thr)
    {
      ReleaseSRWLockExclusive (&srwlock);
      res = WaitForSingleObject (sync_thr, INFINITE);
      AcquireSRWLockExclusive (&srwlock);
      if (res != WAIT_OBJECT_0)
	debug_printf ("WFSO returned unexpected value %u, %E", res);
    }
  return true;
}

timer_tracker::timer_tracker (clockid_t c, const sigevent *e)
: magic (TT_MAGIC), clock_id (c), timer (NULL), cancel_evt (NULL),
  sync_thr (NULL), interval (0), exp_ts (0), overrun_count_curr (0),
  overrun_count (OVR_DISARMED)
{
  srwlock = SRWLOCK_INIT;
  if (e != NULL)
    evp = *e;
  else
    {
      evp.sigev_notify = SIGEV_SIGNAL;
      evp.sigev_signo = SIGALRM;
      evp.sigev_value.sival_ptr = this;
    }
}

timer_tracker::~timer_tracker ()
{
  AcquireSRWLockExclusive (&srwlock);
  cancel ();
  NtClose (cancel_evt);
  NtClose (sync_thr);
  NtClose (timer);
  magic = 0;
  ReleaseSRWLockExclusive (&srwlock);
}

/* Returns 0 if arming successful, -1 if a signal is already queued.
   If so, it also increments overrun_count.  Only call under lock! */
bool
timer_tracker::arm_overrun_event (LONG64 exp_cnt)
{
  bool ret = (overrun_count != OVR_DISARMED);

  overrun_count += exp_cnt;
  return ret;
}

LONG
timer_tracker::disarm_overrun_event ()
{
  LONG ret;

  AcquireSRWLockExclusive (&srwlock);
  if (overrun_count != OVR_DISARMED)
    {
      LONG64 ov_cnt;

      ov_cnt = overrun_count;
      if (ov_cnt > DELAYTIMER_MAX || ov_cnt < 0)
	overrun_count_curr = DELAYTIMER_MAX;
      else
	overrun_count_curr = ov_cnt;
      ret = overrun_count_curr;
      overrun_count = OVR_DISARMED;
    }
  ReleaseSRWLockExclusive (&srwlock);
  return ret;
}

static void *
notify_thread_wrapper (void *arg)
{
  timer_tracker *tt = (timer_tracker *) arg;
  sigevent_t *evt = tt->sigevt ();
  void * (*notify_func) (void *) = (void * (*) (void *))
				   evt->sigev_notify_function;

  tt->disarm_overrun_event ();
  return notify_func (evt->sigev_value.sival_ptr);
}

DWORD
timer_tracker::thread_func ()
{
  HANDLE w4[2] = { timer, cancel_evt };

  debug_printf ("%p timer armed", this);
  while (1)
    {
      switch (WaitForMultipleObjects (2, w4, FALSE, INFINITE))
	{
	case WAIT_OBJECT_0:
	  debug_printf ("%p timer expired", this);
	  break;
	case WAIT_OBJECT_0 + 1:
	  debug_printf ("%p timer disarmed, %E", this);
	  goto out;
	default:
	  debug_printf ("%p wait failed, %E", this);
	  continue;
	}
      AcquireSRWLockExclusive (&srwlock);
      /* Make sure we haven't been abandoned and/or disarmed in the meantime */
      if (exp_ts == 0 && interval == 0)
	{
	  ReleaseSRWLockExclusive (&srwlock);
	  goto out;
	}
      LONG64 exp_cnt = 0;
      if (interval)
	{
	  /* Compute expiration count. */
	  LONG64 now = get_clock_now ();
	  LONG64 ts = get_exp_ts ();

	  /* Make concessions for unexact realtime clock */
	  if (ts > now)
	    ts = now - 1;
	  exp_cnt = (now - ts + interval - 1) / interval;
	  ts += interval * exp_cnt;
	  set_exp_ts (ts);
	  /* NtSetTimer allows periods of up to 24 days only.  If the time
	     is longer, we set the timer up as one-shot timer for each
	     interval.  Restart timer here with new due time. */
	  if (interval > INT_MAX * (NS100PERSEC / MSPERSEC))
	    {
	      BOOLEAN Resume = (clock_id == CLOCK_REALTIME_ALARM
				|| clock_id == CLOCK_BOOTTIME_ALARM);
	      LARGE_INTEGER DueTime = { QuadPart: -interval };

	      NtSetTimer (timer, &DueTime, NULL, NULL, Resume, 0, NULL);
	    }
	}
      switch (evp.sigev_notify)
	{
	case SIGEV_SIGNAL:
	  {
	    if (arm_overrun_event (exp_cnt))
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
	    if (arm_overrun_event (exp_cnt))
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
      /* one-shot timer? */
      if (!interval)
	{
	  memset (&time_spec, 0, sizeof time_spec);
	  exp_ts = 0;
	  overrun_count = OVR_DISARMED;
	  ReleaseSRWLockExclusive (&srwlock);
	  goto out;
	}
      ReleaseSRWLockExclusive (&srwlock);
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

int
timer_tracker::gettime (itimerspec *curr_value, bool lock)
{
  if (lock)
    {
      AcquireSRWLockExclusive (&srwlock);
      if (!is_timer_tracker ())
	{
	  ReleaseSRWLockExclusive (&srwlock);
	  return -EINVAL;
	}
    }
  if (!time_spec.it_value.tv_sec && !time_spec.it_value.tv_nsec)
    memset (curr_value, 0, sizeof (*curr_value));
  else
    {
      LONG64 next_relative_exp = get_exp_ts () - get_clock_now ();
      curr_value->it_value.tv_sec = next_relative_exp / NS100PERSEC;
      next_relative_exp -= curr_value->it_value.tv_sec * NS100PERSEC;
      curr_value->it_value.tv_nsec = next_relative_exp
				     * (NSPERSEC / NS100PERSEC);
      curr_value->it_interval = time_spec.it_interval;
    }
  if (lock)
    ReleaseSRWLockExclusive (&srwlock);
  return 0;
}

int
timer_tracker::settime (int flags, const itimerspec *new_value,
			itimerspec *old_value)
{
  int ret = -1;

  __try
    {
      if (!new_value || !valid_timespec (new_value->it_value)
	  || !valid_timespec (new_value->it_interval))
	{
	  ret = -EINVAL;
	  __leave;
	}

      AcquireSRWLockExclusive (&srwlock);
      if (!is_timer_tracker ())
	{
	  ReleaseSRWLockExclusive (&srwlock);
	  ret = -EINVAL;
	  __leave;
	}

      if (old_value)
	gettime (old_value, false);

      cancel ();
      if (!new_value->it_value.tv_sec && !new_value->it_value.tv_nsec)
	{
	  memset (&time_spec, 0, sizeof time_spec);
	  interval = 0;
	  exp_ts = 0;
	}
      else
	{
	  LONG64 ts;
	  LARGE_INTEGER DueTime;
	  BOOLEAN Resume;
	  LONG Period;
	  NTSTATUS status;

	  if (!timer)
	    {
	      OBJECT_ATTRIBUTES attr;

	      InitializeObjectAttributes (&attr, NULL, 0, NULL, NULL);
	      status = NtCreateEvent (&cancel_evt, EVENT_ALL_ACCESS, &attr,
				      NotificationEvent, FALSE);
	      if (!NT_SUCCESS (status))
		{
		  ret = -geterrno_from_nt_status (status);
		  __leave;
		}
	      status = NtCreateEvent (&sync_thr, EVENT_ALL_ACCESS, &attr,
				      NotificationEvent, FALSE);
	      if (!NT_SUCCESS (status))
		{
		  NtClose (cancel_evt);
		  cancel_evt = NULL;
		  ret = -geterrno_from_nt_status (status);
		  __leave;
		}
	      status = NtCreateTimer (&timer, TIMER_ALL_ACCESS, &attr,
				      SynchronizationTimer);
	      if (!NT_SUCCESS (status))
		{
		  NtClose (cancel_evt);
		  NtClose (sync_thr);
		  cancel_evt = sync_thr = NULL;
		  ret = -geterrno_from_nt_status (status);
		  __leave;
		}
	    }
	  ResetEvent (cancel_evt);
	  ResetEvent (sync_thr);
	  NtCancelTimer (timer, NULL);
	  /* Convert incoming itimerspec into 100ns interval and timestamp */
	  interval = new_value->it_interval.tv_sec * NS100PERSEC
		      + (new_value->it_interval.tv_nsec
			 + (NSPERSEC / NS100PERSEC) - 1)
			/ (NSPERSEC / NS100PERSEC);
	  ts = new_value->it_value.tv_sec * NS100PERSEC
	       + (new_value->it_value.tv_nsec + (NSPERSEC / NS100PERSEC) - 1)
		 / (NSPERSEC / NS100PERSEC);
	  if (flags & TIMER_ABSTIME)
	    {
	      if (clock_id == CLOCK_REALTIME)
		DueTime.QuadPart = ts + FACTOR;
	      else /* non-REALTIME clocks require relative DueTime. */
		{
		  DueTime.QuadPart = get_clock_now () - ts;
		  /* If the timestamp was earlier than now, compute number
		     of expirations and offset DueTime to expire immediately. */
		  if (DueTime.QuadPart >= 0)
		    DueTime.QuadPart = -1LL;
		}
	    }
	  else
	    {
	      /* Keep relative timestamps relative for the timer, but store the
		 expiry timestamp absolute for the timer thread. */
	      DueTime.QuadPart = -ts;
	      ts += get_clock_now ();
	    }
	  set_exp_ts (ts);
	  time_spec = *new_value;
	  overrun_count_curr = 0;
	  overrun_count = OVR_DISARMED;
	  /* Note: Advanced Power Settings -> Sleep -> Allow Wake Timers
		   since W10 1709 */
	  Resume = (clock_id == CLOCK_REALTIME_ALARM
		    || clock_id == CLOCK_BOOTTIME_ALARM);
	  if (interval > INT_MAX * (NS100PERSEC / MSPERSEC))
	    Period = 0;
	  else
	    Period = (interval + (NS100PERSEC / MSPERSEC) - 1)
		     / (NS100PERSEC / MSPERSEC);
	  status = NtSetTimer (timer, &DueTime, NULL, NULL, Resume, Period,
			       NULL);
	  if (!NT_SUCCESS (status))
	    {
	      memset (&time_spec, 0, sizeof time_spec);
	      interval = 0;
	      exp_ts = 0;
	      ret = -geterrno_from_nt_status (status);
	      __leave;
	    }
	  new cygthread (timer_thread, this, "itimer", sync_thr);
	}
      ReleaseSRWLockExclusive (&srwlock);
      ret = 0;
    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

/* The timers are stored on the system heap in order to avoid accidental
   leaking of timer ids into the child process. */
#define cnew(name, ...) \
  ({ \
    void* ptr = (void*) HeapAlloc (GetProcessHeap (), 0, sizeof (name)); \
    ptr ? new (ptr) name (__VA_ARGS__) : NULL; \
  })

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
	  __leave;
	}

      if (clock_id >= MAX_CLOCKS)
	{
	  set_errno (EINVAL);
	  __leave;
	}

      *timerid = (timer_t) cnew (timer_tracker, clock_id, evp);
      if (!*timerid)
	__seterrno ();
      else
	ret = 0;
    }
  __except (EFAULT) {}
  __endtry
  return ret;
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
	  __leave;
	}

      ret = tt->gettime (ovalue, true);
      if (ret < 0)
	{
	  set_errno (-ret);
	  ret = -1;
	}
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
      if (ret < 0)
	{
	  set_errno (-ret);
	  ret = -1;
	}
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
      if (!in_tt->is_timer_tracker () || in_tt == &itimer_tracker)
	{
	  set_errno (EINVAL);
	  __leave;
	}
      delete in_tt;
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
      ret = timer_settime ((timer_t) &itimer_tracker, 0,
			   &spec_value, &spec_ovalue);
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
	  ret = timer_gettime ((timer_t) &itimer_tracker, &spec_ovalue);
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
 timer_settime ((timer_t) &itimer_tracker, 0, &newt, &oldt);
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
 timer_settime ((timer_t) &itimer_tracker, 0, &timer, &otimer);
 useconds_t ret = otimer.it_value.tv_sec * USPERSEC
		  + (otimer.it_value.tv_nsec + (NSPERSEC/USPERSEC) - 1)
		    / (NSPERSEC/USPERSEC);
 syscall_printf ("%d = ualarm(%ld , %ld)", ret, value, interval);
 return ret;
}
