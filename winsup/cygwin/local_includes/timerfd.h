/* timerfd.h: Define timerfd classes

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __TIMERFD_H__
#define __TIMERFD_H__

#include "clock.h"
#include "ntdll.h"

class timerfd_shared
{
  clockid_t _clockid;		/* clockid */
  struct itimerspec _time_spec;	/* original incoming itimerspec */
  LONG64 _exp_ts;		/* start timestamp or next expire timestamp
				   in 100ns */
  LONG64 _interval;		/* timer interval in 100ns */
  LONG64 _expiration_count;	/* expiry counter */
  int _flags;			/* settime flags */
  DWORD _tc_time;		/* timestamp of the last WM_TIMECHANGE msg */

  /* read access methods */
  LONG64 get_clock_now () const { return get_clock (_clockid)->n100secs (); }
  struct itimerspec &time_spec () { return _time_spec; }
  int get_flags () const { return _flags; }
  void set_flags (int nflags) { _flags = nflags; }

  /* write access methods */
  void set_clockid (clockid_t clock_id) { _clockid = clock_id; }
  void increment_expiration_count (LONG64 add)
    { InterlockedAdd64 (&_expiration_count, add); }
  void set_expiration_count (LONG64 newval)
    { InterlockedExchange64 (&_expiration_count, newval); }
  LONG64 reset_expiration_count ()
    { return InterlockedExchange64 (&_expiration_count, 0); }
  int arm_timer (int, const struct itimerspec *);
  int disarm_timer ()
    {
      memset (&_time_spec, 0, sizeof _time_spec);
      _exp_ts = 0;
      _interval = 0;
      /* _flags = 0;  DON'T DO THAT.  Required for TFD_TIMER_CANCEL_ON_SET */
      return 0;
    }
  void set_exp_ts (LONG64 ts) { _exp_ts = ts; }

  friend class timerfd_tracker;
};

class timerfd_tracker		/* cygheap! */
{
  /* Shared handles */
  HANDLE tfd_shared_hdl;	/* handle to shared mem */
  HANDLE _access_mtx;		/* controls access to shared data */
  HANDLE _arm_evt;		/* settimer sets event when timer is armed,
				   unsets event when timer gets disarmed. */
  HANDLE _disarm_evt;		/* settimer sets event when timer is armed,
				   unsets event when timer gets disarmed. */
  HANDLE _timer;		/* SynchronizationTimer */
  HANDLE _expired_evt;		/* Signal if timer expired, Unsignal on read. */
  /* Process-local handles */
  HANDLE cancel_evt;		/* Signal thread to exit. */
  HANDLE sync_thr;		/* cygthread sync object. */
  /* pointer to shared timerfd, misc */
  timerfd_shared *tfd_shared;	/* pointer to shared mem, needs
				   NtMapViewOfSection in each new process. */
  LONG instance_count;		/* each open fd increments this.
				   If 0 -> cancel thread.  */
  DWORD winpid;			/* This is used @ fork/exec time to know if
				   this tracker already has been fixed up. */
  HWND window;			/* window handle */
  ATOM atom;			/* window class */

  void create_timechange_window ();
  void delete_timechange_window ();
  void handle_timechange_window ();

  bool dtor ();

  bool enter_critical_section ()
    {
      return (WaitForSingleObject (_access_mtx, INFINITE) & ~WAIT_ABANDONED_0)
	      == WAIT_OBJECT_0;
    }
  /* A version that honors a cancel event, for use in thread_func. */
  int enter_critical_section_cancelable ();
  void leave_critical_section ()
    {
      ReleaseMutex (_access_mtx);
    }

  HANDLE arm_evt () const { return _arm_evt; }
  HANDLE disarm_evt () const { return _disarm_evt; }
  HANDLE timer () const { return _timer; }
  HANDLE expired_evt () const { return _expired_evt; }
  void timer_expired () { SetEvent (_expired_evt); }
  int arm_timer (int flags, const struct itimerspec *new_value);
  int disarm_timer ()
    {
      ResetEvent (_arm_evt);
      tfd_shared->disarm_timer ();
      NtCancelTimer (timer (), NULL);
      SetEvent (_disarm_evt);
      return 0;
    }
  void timer_expired () const { timer_expired (); }

  LONG64 expiration_count () const { return tfd_shared->_expiration_count; }
  void increment_expiration_count (LONG64 add) const
    { tfd_shared->increment_expiration_count (add); }
  void set_expiration_count (LONG64 exp_cnt) const
    { tfd_shared->set_expiration_count ((LONG64) exp_cnt); }
  LONG64 read_and_reset_expiration_count ()
    {
      LONG64 ret = tfd_shared->reset_expiration_count ();
      if (ret)
	ResetEvent (_expired_evt);
      return ret;
    }

  struct timespec it_value () const
    { return tfd_shared->time_spec ().it_value; }
  struct timespec it_interval () const
    { return tfd_shared->time_spec ().it_interval; }

  void set_clockid (clockid_t clock_id) { tfd_shared->set_clockid (clock_id); }
  clock_t get_clockid () const { return tfd_shared->_clockid; }
  LONG64 get_clock_now () const { return tfd_shared->get_clock_now (); }
  struct itimerspec &time_spec () { return tfd_shared->time_spec (); }
  LONG64 get_exp_ts () const { return tfd_shared->_exp_ts; }
  LONG64 get_interval () const { return tfd_shared->_interval; }
  void set_interval (LONG64 intv) { tfd_shared->_interval = intv; }
  int get_flags () const { return tfd_shared->get_flags (); }
  void set_flags (int nflags) { tfd_shared->set_flags (nflags); }
  DWORD tc_time () const { return tfd_shared->_tc_time; }
  void set_tc_time (DWORD new_time) { tfd_shared->_tc_time = new_time; }

  void set_exp_ts (LONG64 ts) const { tfd_shared->set_exp_ts (ts); }
  LONG decrement_instances () { return InterlockedDecrement (&instance_count); }

 public:
  void *operator new (size_t, void *p) __attribute__ ((nothrow)) {return p;}
  timerfd_tracker ()
  : tfd_shared_hdl (NULL), _access_mtx (NULL), _arm_evt (NULL),
    _disarm_evt (NULL), cancel_evt (NULL), sync_thr (NULL), tfd_shared (NULL),
    instance_count (1), winpid (0), window (NULL), atom (0) {}

  void init_fixup_after_fork_exec ();
  void fixup_after_fork_exec (bool);
  void fixup_after_fork ()
    {
      init_fixup_after_fork_exec ();
      fixup_after_fork_exec (false);
    }
  void fixup_after_exec () { fixup_after_fork_exec (true); }

  void dup () { InterlockedIncrement (&instance_count); }
  HANDLE get_timerfd_handle () const { return expired_evt (); }
  LONG64 wait (bool);
  int ioctl_set_ticks (uint64_t);

  int create (clockid_t);
  int gettime (struct itimerspec *);
  int settime (int, const struct itimerspec *, struct itimerspec *);

  static void dtor (timerfd_tracker *);
  DWORD thread_func ();
};

#endif /* __TIMERFD_H__ */
