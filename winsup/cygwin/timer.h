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

  clockid_t clock_id;
  sigevent evp;
  timespec it_interval;
  HANDLE hcancel;
  HANDLE syncthread;
  int64_t interval_us;
  int64_t sleepto_us;
  LONG event_running;
  LONG overrun_count_curr;
  LONG64 overrun_count;

  bool cancel ();

 public:
  timer_tracker (clockid_t, const sigevent *);
  ~timer_tracker ();
  inline bool is_timer_tracker () const { return magic == TT_MAGIC; }
  inline sigevent_t *sigevt () { return &evp; }
  inline int getoverrun () const { return overrun_count_curr; }

  void gettime (itimerspec *);
  int settime (int, const itimerspec *, itimerspec *);
  int clean_and_unhook ();
  LONG arm_event ();
  LONG disarm_event ();

  DWORD thread_func ();
  static void fixup_after_fork ();
};

#endif /* __TIMER_H__ */
