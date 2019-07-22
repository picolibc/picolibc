/* timer.h: Define class timer_tracker, base class for posix timers

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __POSIX_TIMER_H__
#define __POSIX_TIMER_H__

#define TT_MAGIC 0x513e4a1c
class timer_tracker
{
  unsigned magic;
  SRWLOCK srwlock;
  clockid_t clock_id;
  sigevent evp;
  struct itimerspec time_spec;
  HANDLE timer;
  HANDLE cancel_evt;
  HANDLE sync_thr;
  LONG64 interval;
  LONG64 exp_ts;
  LONG overrun_count_curr;
  LONG64 overrun_count;

  bool cancel ();

 public:
  void *operator new (size_t, void *p) __attribute__ ((nothrow)) {return p;}
  void operator delete (void *p) { HeapFree (GetProcessHeap (), 0, p); }
  timer_tracker (clockid_t, const sigevent *);
  ~timer_tracker ();
  inline bool is_timer_tracker () const { return magic == TT_MAGIC; }
  inline sigevent_t *sigevt () { return &evp; }
  inline int getoverrun () const { return overrun_count_curr; }

  LONG64 get_clock_now () const { return get_clock (clock_id)->n100secs (); }
  LONG64 get_exp_ts () const { return exp_ts; }
  LONG64 get_interval () const { return interval; }
  void set_exp_ts (LONG64 ts) { exp_ts = ts; }

  bool arm_overrun_event (LONG64);
  LONG disarm_overrun_event ();

  int gettime (itimerspec *, bool);
  int settime (int, const itimerspec *, itimerspec *);

  DWORD thread_func ();
  static void fixup_after_fork ();
};

#endif /* __POSIX_TIMER_H__ */
