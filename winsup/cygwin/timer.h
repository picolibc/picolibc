/* timer.h: Define class timer_tracker, base class for posix timers

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __TIMER_H__
#define __TIMER_H__

#define TT_MAGIC 0x513e4a1c
class timer_tracker
{
  unsigned magic;
  timer_tracker *next;
  LONG instance_count;

  clockid_t clock_id;
  sigevent evp;
  timespec it_interval;
  bool deleting;
  HANDLE hcancel;
  HANDLE syncthread;
  HANDLE timerfd_event;
  int64_t interval_us;
  int64_t sleepto_us;
  LONG event_running;
  LONG64 overrun_count_curr;
  LONG64 overrun_count;

  bool cancel ();
  LONG decrement_instances ();
  int clean_and_unhook ();
  LONG64 _disarm_event ();

 public:
  timer_tracker (clockid_t, const sigevent *, bool);
  ~timer_tracker ();
  inline bool is_timer_tracker () const { return magic == TT_MAGIC; }

  void increment_instances ();
  LONG64 wait (bool nonblocking);
  HANDLE get_timerfd_handle () const { return timerfd_event; }

  inline sigevent_t *sigevt () { return &evp; }
  inline LONG64 getoverrun () const { return overrun_count_curr; }

  void gettime (itimerspec *);
  int settime (int, const itimerspec *, itimerspec *);
  LONG arm_event ();
  unsigned int disarm_event ();

  DWORD thread_func ();
  static void fixup_after_fork ();
  static int close (timer_tracker *tt);
};

#endif /* __TIMER_H__ */
