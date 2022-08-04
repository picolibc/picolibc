/* timerfd.cc: timerfd helper classes

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "path.h"
#include "fhandler.h"
#include "pinfo.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygerrno.h"
#include <sys/timerfd.h>
#include "timerfd.h"

#define TFD_CANCEL_FLAGS (TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET)

/* Unfortunately MsgWaitForMultipleObjectsEx does not receive WM_TIMECHANGED
   messages without a window defined in this process.  Create a hidden window
   for that purpose. */

void
timerfd_tracker::create_timechange_window ()
{
  WNDCLASSW wclass = { 0 };
  WCHAR cname[NAME_MAX];

  __small_swprintf (cname, L"Cygwin.timerfd.%p", this);
  wclass.lpfnWndProc = DefWindowProcW;
  wclass.hInstance = user_data->hmodule;
  wclass.lpszClassName = cname;
  /* This sleep is required on Windows 10 64 bit only, and only when running
     under strace.  One of the child processes inheriting the timerfd
     descriptor will get a STATUS_FLOAT_INEXACT_RESULT exception inside of
     msvcrt.dll.  While this is completely crazy in itself, it's apparently
     some timing problem.  It occurs in 4 out of 5 runs under strace only.
     The sleep is required before calling RegisterClassW.  Moving it before
     CreateWindowExW does not work.  What the heck? */
  if (being_debugged ())
    Sleep (1L);
  atom = RegisterClassW (&wclass);
  if (!atom)
    debug_printf ("RegisterClass %E");
  else
    {
      window = CreateWindowExW (0, cname, cname, WS_POPUP, 0, 0, 0, 0,
				NULL, NULL, user_data->hmodule, NULL);
      if (!window)
	debug_printf ("CreateWindowEx %E");
    }
}

void
timerfd_tracker::delete_timechange_window ()
{
  if (window)
    DestroyWindow (window);
  if (atom)
    UnregisterClassW ((LPWSTR) (uintptr_t) atom, user_data->hmodule);
}

void
timerfd_tracker::handle_timechange_window ()
{
  MSG msg;

  while (PeekMessageW (&msg, NULL, 0, 0, PM_REMOVE | PM_QS_POSTMESSAGE)
	 && msg.message != WM_QUIT)
    {
      DispatchMessageW (&msg);
      if (msg.message == WM_TIMECHANGE
	  && get_clockid () == CLOCK_REALTIME
	  && (get_flags () & TFD_CANCEL_FLAGS) == TFD_CANCEL_FLAGS
	  && enter_critical_section ())
	{
	  /* make sure to handle each WM_TIMECHANGE only once! */
	  if (msg.time != tc_time ())
	    {
	      set_expiration_count (-1LL);
	      disarm_timer ();
	      timer_expired ();
	      set_tc_time (msg.time);
	    }
	  leave_critical_section ();
	}
    }
}

/* Like enter_critical_section, but returns -1 on a cancel event. */
int
timerfd_tracker::enter_critical_section_cancelable ()
{
  HANDLE w[2] = { cancel_evt, _access_mtx };
  DWORD waitret = WaitForMultipleObjects (2, w, FALSE, INFINITE);

  switch (waitret)
    {
    case WAIT_OBJECT_0:
      return -1;
    case WAIT_OBJECT_0 + 1:
    case WAIT_ABANDONED_0 + 1:
      return 1;
    default:
      return 0;
    }
}

DWORD
timerfd_tracker::thread_func ()
{
  /* Outer loop: Is the timer armed?  If not, wait for it. */
  HANDLE armed[2] = { arm_evt (),
		      cancel_evt };

  create_timechange_window ();
  while (1)
    {
      switch (MsgWaitForMultipleObjectsEx (2, armed, INFINITE, QS_POSTMESSAGE,
					   MWMO_INPUTAVAILABLE))
	{
	case WAIT_OBJECT_0:
	  break;
	case WAIT_OBJECT_0 + 1:
	  goto canceled;
	case WAIT_OBJECT_0 + 2:
	  handle_timechange_window ();
	  continue;
	default:
	  continue;
	}

      /* Inner loop: Timer expired?  If not, wait for it. */
      HANDLE expired[3] = { timer (),
			    disarm_evt (),
			    cancel_evt };

      while (1)
	{
	  switch (MsgWaitForMultipleObjectsEx (3, expired, INFINITE,
					       QS_POSTMESSAGE,
					       MWMO_INPUTAVAILABLE))
	    {
	    case WAIT_OBJECT_0:
	      break;
	    case WAIT_OBJECT_0 + 1:
	      goto disarmed;
	    case WAIT_OBJECT_0 + 2:
	      goto canceled;
	    case WAIT_OBJECT_0 + 3:
	      handle_timechange_window ();
	      continue;
	    default:
	      continue;
	    }

	  int ec = enter_critical_section_cancelable ();
	  if (ec < 0)
	    goto canceled;
	  else if (!ec)
	    continue;
	  /* Make sure we haven't been abandoned and/or disarmed
	     in the meantime */
	  if (expiration_count () == -1LL
	      || IsEventSignalled (disarm_evt ()))
	    {
	      leave_critical_section ();
	      goto disarmed;
	    }
	  /* One-shot timer? */
	  if (!get_interval ())
	    {
	      /* Set expiration count, disarm timer */
	      increment_expiration_count (1);
	      disarm_timer ();
	    }
	  else
	    {
	      /* Compute expiration count. */
	      LONG64 now = get_clock_now ();
	      LONG64 ts = get_exp_ts ();
	      LONG64 exp_cnt;

	      /* Make concessions for unexact realtime clock */
	      if (ts > now)
		ts = now - 1;
	      exp_cnt = (now - ts + get_interval () - 1) / get_interval ();
	      increment_expiration_count (exp_cnt);
	      ts += get_interval () * exp_cnt;
	      /* Set exp_ts to current timestamp.  Make sure exp_ts ends up
		 bigger than "now" and fix expiration count as required */
	      while (ts <= (now = get_clock_now ()))
		{
		  exp_cnt = (now - ts + get_interval () - 1) / get_interval ();
		  increment_expiration_count (exp_cnt);
		  ts += get_interval () * exp_cnt;
		}
	      set_exp_ts (ts);
	      /* NtSetTimer allows periods of up to 24 days only.  If the time
		 is longer, we set the timer up as one-shot timer for each
		 interval.  Restart timer here with new due time. */
	      if (get_interval () > INT_MAX * (NS100PERSEC / MSPERSEC))
		{
		  BOOLEAN Resume = (get_clockid () == CLOCK_REALTIME_ALARM
				    || get_clockid () == CLOCK_BOOTTIME_ALARM);
		  LARGE_INTEGER DueTime = { QuadPart: -get_interval () };

		  NtSetTimer (timer (), &DueTime, NULL, NULL, Resume, 0, NULL);
		}
	    }
	  /* Arm the expiry object */
	  timer_expired ();
	  leave_critical_section ();
	}
disarmed:
      ;
    }

canceled:
  delete_timechange_window ();
  /* automatically return the cygthread to the cygthread pool */
  _my_tls._ctinfo->auto_release ();
  return 0;
}

static DWORD
timerfd_thread (VOID *arg)
{
  timerfd_tracker *tt = ((timerfd_tracker *) arg);
  return tt->thread_func ();
}

int
timerfd_tracker::create (clockid_t clock_id)
{
  int ret;
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;

  const ACCESS_MASK access = STANDARD_RIGHTS_REQUIRED
			     | SECTION_MAP_READ | SECTION_MAP_WRITE;
  SIZE_T vsize = wincap.page_size ();
  LARGE_INTEGER sectionsize = { QuadPart: (LONGLONG) wincap.page_size () };

  /* Valid clock? */
  if (!get_clock (clock_id))
    {
      ret = -EINVAL;
      goto err;
    }

  /* Create shared objects */
  InitializeObjectAttributes (&attr, NULL, OBJ_INHERIT, NULL, NULL);
  /* Create shared section */
  status = NtCreateSection (&tfd_shared_hdl, access, &attr, &sectionsize,
			    PAGE_READWRITE, SEC_COMMIT, NULL);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err;
    }
  /* Create access mutex */
  status = NtCreateMutant (&_access_mtx, MUTEX_ALL_ACCESS, &attr, FALSE);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err_close_tfd_shared_hdl;
    }
  /* Create "timer is armed" event, set to "Unsignaled" at creation time */
  status = NtCreateEvent (&_arm_evt, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err_close_access_mtx;
    }
  /* Create "timer is disarmed" event, set to "Signaled" at creation time */
  status = NtCreateEvent (&_disarm_evt, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, TRUE);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err_close_arm_evt;
    }
  /* Create timer */
  status = NtCreateTimer (&_timer, TIMER_ALL_ACCESS, &attr,
			  SynchronizationTimer);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err_close_disarm_evt;
    }
  /* Create "timer expired" semaphore */
  status = NtCreateEvent (&_expired_evt, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err_close_timer;
    }
  /* Create process-local cancel event for this processes timer thread
     (has to be recreated after fork/exec)*/
  InitializeObjectAttributes (&attr, NULL, 0, NULL, NULL);
  status = NtCreateEvent (&cancel_evt, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err_close_expired_evt;
    }
  /* Create sync event for this processes timer thread */
  status = NtCreateEvent (&sync_thr, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err_close_cancel_evt;
    }
  /* Create section mapping (has to be recreated after fork/exec) */
  tfd_shared = NULL;
  status = NtMapViewOfSection (tfd_shared_hdl, NtCurrentProcess (),
			       (void **) &tfd_shared, 0, vsize, NULL,
			       &vsize, ViewShare, 0, PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err_close_sync_thr;
    }
  /* Initialize clock id */
  set_clockid (clock_id);
  /* Set our winpid for fixup_after_fork_exec */
  winpid = GetCurrentProcessId ();
  /* Start timerfd thread */
  new cygthread (timerfd_thread, this, "timerfd", sync_thr);
  return 0;

err_close_sync_thr:
  NtClose (sync_thr);
err_close_cancel_evt:
  NtClose (cancel_evt);
err_close_expired_evt:
  NtClose (_expired_evt);
err_close_timer:
  NtClose (_timer);
err_close_disarm_evt:
  NtClose (_disarm_evt);
err_close_arm_evt:
  NtClose (_arm_evt);
err_close_access_mtx:
  NtClose (_access_mtx);
err_close_tfd_shared_hdl:
  NtClose (tfd_shared_hdl);
err:
  return ret;
}

/* Return true if this was the last instance of a timerfd, process-wide,
   false otherwise.  Basically this is a destructor, but one which may
   notify the caller NOT to deleted the object. */
bool
timerfd_tracker::dtor ()
{
  if (!enter_critical_section ())
    return false;
  if (decrement_instances () > 0)
    {
      leave_critical_section ();
      return false;
    }
  if (cancel_evt)
    SetEvent (cancel_evt);
  if (sync_thr)
    {
      WaitForSingleObject (sync_thr, INFINITE);
      NtClose (sync_thr);
    }
  leave_critical_section ();
  if (tfd_shared)
    NtUnmapViewOfSection (NtCurrentProcess (), tfd_shared);
  if (cancel_evt)
    NtClose (cancel_evt);
  NtClose (tfd_shared_hdl);
  NtClose (_expired_evt);
  NtClose (_timer);
  NtClose (_disarm_evt);
  NtClose (_arm_evt);
  NtClose (_access_mtx);
  return true;
}

void
timerfd_tracker::dtor (timerfd_tracker *tfd)
{
  if (tfd->dtor ())
    cfree (tfd);
}

int
timerfd_tracker::ioctl_set_ticks (uint64_t new_exp_cnt)
{
  LONG64 exp_cnt = (LONG64) new_exp_cnt;
  if (exp_cnt == 0 || exp_cnt == -1LL)
    return -EINVAL;
  if (!enter_critical_section ())
    return -EBADF;
  set_expiration_count (exp_cnt);
  timer_expired ();
  leave_critical_section ();
  return 0;
}

void
timerfd_tracker::init_fixup_after_fork_exec ()
{
  /* Run this only if this is the first call, or all previous calls
     came from close_on_exec descriptors */
  if (winpid == GetCurrentProcessId ())
    return;
  tfd_shared = NULL;
  cancel_evt = NULL;
  sync_thr = NULL;
}

void
timerfd_tracker::fixup_after_fork_exec (bool execing)
{
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;
  SIZE_T vsize = wincap.page_size ();

  /* Run this only once per process */
  if (winpid == GetCurrentProcessId ())
    return;
  /* Recreate shared section mapping */
  tfd_shared = NULL;
  status = NtMapViewOfSection (tfd_shared_hdl, NtCurrentProcess (),
			       (PVOID *) &tfd_shared, 0, vsize, NULL,
			       &vsize, ViewShare, 0, PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    api_fatal ("Can't recreate shared timerfd section during %s, status %y!",
	       execing ? "execve" : "fork", status);
  /* Create cancel event for this processes timer thread */
  InitializeObjectAttributes (&attr, NULL, 0, NULL, NULL);
  status = NtCreateEvent (&cancel_evt, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    api_fatal ("Can't recreate timerfd cancel event during %s, status %y!",
	       execing ? "execve" : "fork", status);
  /* Create sync event for this processes timer thread */
  InitializeObjectAttributes (&attr, NULL, 0, NULL, NULL);
  status = NtCreateEvent (&sync_thr, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    api_fatal ("Can't recreate timerfd sync event during %s, status %y!",
	       execing ? "execve" : "fork", status);
  /* Set winpid so we don't run this twice */
  winpid = GetCurrentProcessId ();
  new cygthread (timerfd_thread, this, "timerfd", sync_thr);
}

LONG64
timerfd_tracker::wait (bool nonblocking)
{
  HANDLE w4[2] = { get_timerfd_handle (), NULL };
  LONG64 ret;

  wait_signal_arrived here (w4[1]);
repeat:
  switch (WaitForMultipleObjects (2, w4, FALSE, nonblocking ? 0 : INFINITE))
    {
    case WAIT_OBJECT_0:		/* timer event */
      if (!enter_critical_section ())
	ret = -EIO;
      else
	{
	  ret = read_and_reset_expiration_count ();
	  leave_critical_section ();
	  switch (ret)
	    {
	    case -1:	/* TFD_TIMER_CANCEL_ON_SET */
	      ret = -ECANCELED;
	      break;
	    case 0:	/* Another read was quicker. */
	      if (!nonblocking)
		goto repeat;
	      ret = -EAGAIN;
	      break;
	    default:	/* Return (positive) expiration count. */
	      if (ret < 0)
		ret = INT64_MAX;
	      break;
	    }
	}
      break;
    case WAIT_OBJECT_0 + 1:	/* signal */
      if (_my_tls.call_signal_handler ())
        goto repeat;
      ret = -EINTR;
      break;
    case WAIT_TIMEOUT:
      ret = -EAGAIN;
      break;
    default:
      ret = -geterrno_from_win_error ();
      break;
    }
  return ret;
}

int
timerfd_tracker::gettime (struct itimerspec *curr_value)
{
  int ret = 0;

  __try
    {
      if (!enter_critical_section ())
	{
	  ret = -EBADF;
	  __leave;
	}
    }
  __except (NO_ERROR)
    {
      return -EFAULT;
    }
  __endtry

  __try
    {
      if (IsEventSignalled (disarm_evt ()))
	*curr_value = time_spec ();
      else
	{
	  LONG64 next_relative_exp = get_exp_ts () - get_clock_now ();
	  curr_value->it_value.tv_sec = next_relative_exp / NS100PERSEC;
	  next_relative_exp -= curr_value->it_value.tv_sec * NS100PERSEC;
	  curr_value->it_value.tv_nsec = next_relative_exp
					 * (NSPERSEC / NS100PERSEC);
	  curr_value->it_interval = time_spec ().it_interval;
	}
      ret = 0;
    }
  __except (NO_ERROR)
    {
      ret = -EFAULT;
    }
  __endtry
  leave_critical_section ();
  return ret;
}

int
timerfd_tracker::arm_timer (int flags, const struct itimerspec *new_value)
{
  LONG64 interval;
  LONG64 ts;
  NTSTATUS status;
  LARGE_INTEGER DueTime;
  BOOLEAN Resume;
  LONG Period;

  ResetEvent (disarm_evt ());

  /* Convert incoming itimerspec into 100ns interval and timestamp */
  interval = new_value->it_interval.tv_sec * NS100PERSEC
	     + (new_value->it_interval.tv_nsec + (NSPERSEC / NS100PERSEC) - 1)
	       / (NSPERSEC / NS100PERSEC);
  ts = new_value->it_value.tv_sec * NS100PERSEC
	    + (new_value->it_value.tv_nsec + (NSPERSEC / NS100PERSEC) - 1)
	      / (NSPERSEC / NS100PERSEC);
  set_flags (flags);
  if (flags & TFD_TIMER_ABSTIME)
    {
      if (get_clockid () == CLOCK_REALTIME)
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
  time_spec () = *new_value;
  set_exp_ts (ts);
  set_interval (interval);
  read_and_reset_expiration_count ();
  /* Note: Advanced Power Settings -> Sleep -> Allow Wake Timers
	   since W10 1709 */
  Resume = (get_clockid () == CLOCK_REALTIME_ALARM
	    || get_clockid () == CLOCK_BOOTTIME_ALARM);
  if (interval > INT_MAX * (NS100PERSEC / MSPERSEC))
    Period = 0;
  else
    Period = (interval + (NS100PERSEC / MSPERSEC) - 1)
	     / (NS100PERSEC / MSPERSEC);
  status = NtSetTimer (timer (), &DueTime, NULL, NULL, Resume, Period, NULL);
  if (!NT_SUCCESS (status))
    {
      disarm_timer ();
      return -geterrno_from_nt_status (status);
    }

  SetEvent (arm_evt ());
  return 0;
}

int
timerfd_tracker::settime (int flags, const struct itimerspec *new_value,
			  struct itimerspec *old_value)
{
  int ret = 0;

  __try
    {
      if (!valid_timespec (new_value->it_value)
          || !valid_timespec (new_value->it_interval))
	{
	  ret = -EINVAL;
	  __leave;
	}
      if (!enter_critical_section ())
	{
	  ret = -EBADF;
	  __leave;
	}
      if (old_value && (ret = gettime (old_value)) < 0)
	__leave;
      if (new_value->it_value.tv_sec == 0 && new_value->it_value.tv_nsec == 0)
	ret = disarm_timer ();
      else
	ret = arm_timer (flags, new_value);
      leave_critical_section ();
      ret = 0;
    }
  __except (NO_ERROR)
    {
      ret = -EFAULT;
    }
  __endtry
  return ret;
}
