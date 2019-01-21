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

  __small_swprintf (cname, L"Cygwin.timerfd.%u", winpid);
  wclass.lpfnWndProc = DefWindowProcW;
  wclass.hInstance = GetModuleHandle (NULL);
  wclass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
  wclass.lpszClassName = cname;
  atom = RegisterClassW (&wclass);
  if (!atom)
    debug_printf ("RegisterClass %E");
  else
    {
      window = CreateWindowExW (0, cname, cname, WS_POPUP, 0, 0, 0, 0,
				NULL, NULL, NULL, NULL);
      if (!window)
	debug_printf ("RegisterClass %E");
    }
}

void
timerfd_tracker::delete_timechange_window ()
{
  if (window)
    DestroyWindow (window);
  if (atom)
    UnregisterClassW ((LPWSTR) (uintptr_t) atom, GetModuleHandle (NULL));
}

void
timerfd_tracker::handle_timechange_window ()
{
  MSG msg;

  if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
    {
      DispatchMessageW(&msg);
      if (msg.message == WM_TIMECHANGE
	  && get_clockid () == CLOCK_REALTIME
	  && (flags () & TFD_CANCEL_FLAGS) == TFD_CANCEL_FLAGS
	  && enter_critical_section ())
	{
	  /* make sure to handle each WM_TIMECHANGE only once! */
	  if (msg.time != tc_time ())
	    {
	      set_overrun_count (-1LL);
	      disarm_timer ();
	      timer_expired ();
	      set_tc_time (msg.time);
	    }
	  leave_critical_section ();
	}
    }
}

DWORD
timerfd_tracker::thread_func ()
{
  /* Outer loop: Is the timer armed?  If not, wait for it. */
  HANDLE armed[2] = { tfd_shared->arm_evt (),
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
      /* TODO: TFD_TIMER_CANCEL_ON_SET */
      HANDLE expired[3] = { tfd_shared->timer (),
			    tfd_shared->disarm_evt (),
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

	  if (!enter_critical_section ())
	    continue;
	  /* Make sure we haven't been abandoned and/or disarmed
	     in the meantime */
	  if (overrun_count () == -1LL
	      || IsEventSignalled (tfd_shared->disarm_evt ()))
	    {
	      leave_critical_section ();
	      goto disarmed;
	    }
	  /* One-shot timer? */
	  if (!get_interval ())
	    {
	      /* Set overrun count, disarm timer */
	      increment_overrun_count (1);
	      disarm_timer ();
	    }
	  else
	    {
	      /* Compute overrun count. */
	      LONG64 now = get_clock_now ();
	      LONG64 ts = get_exp_ts ();

	      /* Make concessions for unexact realtime clock */
	      if (ts > now)
		ts = now - 1;
	      increment_overrun_count ((now - ts + get_interval () - 1)
				       / get_interval ());
	      /* Set exp_ts to current timestamp.  Make sure exp_ts ends up
		 bigger than "now" and fix overrun count as required */
	      while ((ts += get_interval ()) <= (now = get_clock_now ()))
		increment_overrun_count ((now - ts + get_interval () - 1)
					 / get_interval ());
	      set_exp_ts (ts);
	      /* NtSetTimer allows periods of up to 24 days only.  If the time
		 is longer, we set the timer up as one-shot timer for each
		 interval.  Restart timer here with new due time. */
	      if (get_interval () > INT_MAX * (NS100PERSEC / MSPERSEC))
		{
		  /* TODO: CLOCK_REALTIME_ALARM / CLOCK_BOOTTIME_ALARM
		     See comment in arm_timer */
		  BOOL Resume = FALSE;
		  LARGE_INTEGER DueTime = { QuadPart: -get_interval () };

		  NtSetTimer (tfd_shared->timer (), &DueTime, NULL, NULL,
			      Resume, 0, NULL);
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
  _my_tls._ctinfo->auto_release ();     /* automatically return the cygthread to the cygthread pool */
  return 0;
}

static DWORD WINAPI
timerfd_thread (VOID *arg)
{
  timerfd_tracker *tt = ((timerfd_tracker *) arg);
  return tt->thread_func ();
}

int
timerfd_shared::create (clockid_t clock_id)
{
  int ret;
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;

  /* Create access mutex */
  InitializeObjectAttributes (&attr, NULL, OBJ_INHERIT, NULL,
			      sec_none.lpSecurityDescriptor);
  status = NtCreateMutant (&_access_mtx, MUTEX_ALL_ACCESS, &attr, FALSE);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err;
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
  instance_count = 1;
  _clockid = clock_id;
  return 0;

err_close_timer:
  NtClose (_timer);
err_close_disarm_evt:
  NtClose (_disarm_evt);
err_close_arm_evt:
  NtClose (_arm_evt);
err_close_access_mtx:
  NtClose (_access_mtx);
err:
  return ret;
}

int
timerfd_tracker::create (clockid_t clock_id)
{
  int ret;
  clk_t *clock;
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;

  const ACCESS_MASK access = STANDARD_RIGHTS_REQUIRED
			     | SECTION_MAP_READ | SECTION_MAP_WRITE;
  SIZE_T vsize = PAGE_SIZE;
  LARGE_INTEGER sectionsize = { QuadPart: PAGE_SIZE };

  /* Valid clock? */
  clock = get_clock (clock_id);
  if (!clock)
    {
      ret = -EINVAL;
      goto err;
    }
  /* Create shared section. */
  InitializeObjectAttributes (&attr, NULL, OBJ_INHERIT, NULL,
			      sec_none.lpSecurityDescriptor);
  status = NtCreateSection (&tfd_shared_hdl, access, &attr,
			    &sectionsize, PAGE_READWRITE,
			    SEC_COMMIT, NULL);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err;
    }
  /* Create section mapping (has to be repeated after fork/exec */
  status = NtMapViewOfSection (tfd_shared_hdl, NtCurrentProcess (),
			       (void **) &tfd_shared, 0, PAGE_SIZE, NULL,
			       &vsize, ViewShare, MEM_TOP_DOWN, PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err_close_tfd_shared_hdl;
    }
  /* Create cancel even for this processes timer thread */
  InitializeObjectAttributes (&attr, NULL, 0, NULL,
			      sec_none_nih.lpSecurityDescriptor);
  status = NtCreateEvent (&cancel_evt, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    {
      ret = -geterrno_from_nt_status (status);
      goto err_unmap_tfd_shared;
    }
  ret = tfd_shared->create (clock_id);
  if (ret < 0)
    goto err_close_cancel_evt;
  winpid = GetCurrentProcessId ();
  new cygthread (timerfd_thread, this, "timerfd", sync_thr);
  return 0;

err_close_cancel_evt:
  NtClose (cancel_evt);
err_unmap_tfd_shared:
  NtUnmapViewOfSection (NtCurrentProcess (), tfd_shared);
err_close_tfd_shared_hdl:
  NtClose (tfd_shared_hdl);
err:
  return ret;
}

/* Return true if this was the last instance of a timerfd, session-wide,
   false otherwise */
bool
timerfd_shared::dtor ()
{
  if (instance_count > 0)
    {
      return false;
    }
  disarm_timer ();
  NtClose (_timer);
  NtClose (_arm_evt);
  NtClose (_disarm_evt);
  NtClose (_expired_evt);
  NtClose (_access_mtx);
  return true;
}

/* Return true if this was the last instance of a timerfd, session-wide,
   false otherwise.  Basically this is a destructor, but one which may
   notify the caller NOT to deleted the object. */
bool
timerfd_tracker::dtor ()
{
  if (enter_critical_section ())
    {
      if (local_instance_count > 0)
	{
	  leave_critical_section ();
	  return false;
	}
      SetEvent (cancel_evt);
      WaitForSingleObject (sync_thr, INFINITE);
      if (tfd_shared->dtor ())
	{
	  NtUnmapViewOfSection (NtCurrentProcess (), tfd_shared);
	  NtClose (tfd_shared_hdl);
	}
      else
	leave_critical_section ();
    }
  NtClose (cancel_evt);
  NtClose (sync_thr);
  return true;
}

void
timerfd_tracker::dtor (timerfd_tracker *tfd)
{
  if (tfd->dtor ())
    cfree (tfd);
}

void
timerfd_tracker::close ()
{
  InterlockedDecrement (&local_instance_count);
  InterlockedDecrement (&tfd_shared->instance_count);
}

void
timerfd_tracker::ioctl_set_ticks (uint64_t ov_cnt)
{
  set_overrun_count (ov_cnt);
  timer_expired ();
}

void
timerfd_tracker::fixup_after_fork_exec (bool execing)
{
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;
  SIZE_T vsize = PAGE_SIZE;

  /* Run this only once per process */
  if (winpid == GetCurrentProcessId ())
    return;
  /* Recreate shared section mapping */
  status = NtMapViewOfSection (tfd_shared_hdl, NtCurrentProcess (),
			       (void **) &tfd_shared, 0, PAGE_SIZE, NULL,
			       &vsize, ViewShare, MEM_TOP_DOWN, PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    api_fatal ("Can't recreate shared timerfd section during %s!",
	       execing ? "execve" : "fork");
  /* Increment global instance count by the number of instances in this
     process */
  InterlockedAdd (&tfd_shared->instance_count, local_instance_count);
  /* Create cancel even for this processes timer thread */
  InitializeObjectAttributes (&attr, NULL, 0, NULL,
			      sec_none_nih.lpSecurityDescriptor);
  status = NtCreateEvent (&cancel_evt, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    api_fatal ("Can't recreate timerfd cancel event during %s!",
	       execing ? "execve" : "fork");
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
	  ret = read_and_reset_overrun_count ();
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
	    default:	/* Return (positive) overrun count. */
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
      LONG64 next_relative_exp = get_exp_ts () - get_clock_now ();
      curr_value->it_value.tv_sec = next_relative_exp / NS100PERSEC;
      next_relative_exp -= curr_value->it_value.tv_sec * NS100PERSEC;
      curr_value->it_value.tv_nsec = next_relative_exp
				     * (NSPERSEC / NS100PERSEC);
      curr_value->it_interval = time_spec ().it_interval;
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

int
timerfd_shared::arm_timer (int flags, const struct itimerspec *new_value)
{
  LONG64 ts;
  NTSTATUS status;
  LARGE_INTEGER DueTime;
  BOOLEAN Resume;
  LONG Period;

  ResetEvent (_disarm_evt);

  /* Convert incoming itimerspec into 100ns interval and timestamp */
  _interval = new_value->it_interval.tv_sec * NS100PERSEC
	      + (new_value->it_interval.tv_nsec + (NSPERSEC / NS100PERSEC) - 1)
		/ (NSPERSEC / NS100PERSEC);
  ts = new_value->it_value.tv_sec * NS100PERSEC
	    + (new_value->it_value.tv_nsec + (NSPERSEC / NS100PERSEC) - 1)
	      / (NSPERSEC / NS100PERSEC);
  _flags = flags;
  if (flags & TFD_TIMER_ABSTIME)
    {
      if (_clockid == CLOCK_REALTIME)
	DueTime.QuadPart = ts + FACTOR;
      else /* non-REALTIME clocks require relative DueTime. */
	{
	  DueTime.QuadPart = get_clock_now () - ts;
	  /* If the timestamp was earlier than now, compute number
	     of overruns and offset DueTime to expire immediately. */
	  if (DueTime.QuadPart >= 0)
	    {
	      LONG64 num_intervals = DueTime.QuadPart / _interval;
	      increment_overrun_count (num_intervals);
	      DueTime.QuadPart = -1LL;
	    }
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
  /* TODO: CLOCK_REALTIME_ALARM / CLOCK_BOOTTIME_ALARM
	   Note: Advanced Power Settings -> Sleep -> Allow Wake Timers
	   since W10 1709 */
  Resume = FALSE;
  if (_interval > INT_MAX * (NS100PERSEC / MSPERSEC))
    Period = 0;
  else
    Period = (_interval + (NS100PERSEC / MSPERSEC) - 1)
	     / (NS100PERSEC / MSPERSEC);
  status = NtSetTimer (timer (), &DueTime, NULL, NULL, Resume, Period, NULL);
  if (!NT_SUCCESS (status))
    {
      disarm_timer ();
      return -geterrno_from_nt_status (status);
    }

  SetEvent (_arm_evt);
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
