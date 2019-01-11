/* timer.h: Define class timer_tracker, base class for timer handling

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
  long long interval_us;
  long long sleepto_us;

  bool cancel ();

 public:
  timer_tracker (clockid_t, const sigevent *);
  ~timer_tracker ();
  inline bool is_timer_tracker () { return magic == TT_MAGIC; }

  void gettime (itimerspec *);
  int settime (int, const itimerspec *, itimerspec *);
  int clean_and_unhook ();

  DWORD thread_func ();
  static void fixup_after_fork ();
};

#endif /* __TIMER_H__ */
