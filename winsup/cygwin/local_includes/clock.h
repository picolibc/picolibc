/* clock.h: Definitions for clock calculations

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <mmsystem.h>

/* Must be a power of 2. */
#define MAX_CLOCKS		 (16)

/* Conversions for per-process and per-thread clocks */
#define CLOCKID(cid) \
		((cid) % MAX_CLOCKS)
#define PID_TO_CLOCKID(pid) \
		((pid) * MAX_CLOCKS + CLOCK_PROCESS_CPUTIME_ID)
#define CLOCKID_TO_PID(cid) \
		(((cid) - CLOCK_PROCESS_CPUTIME_ID) / MAX_CLOCKS)
#define CLOCKID_IS_PROCESS(cid) \
		(CLOCKID(cid) == CLOCK_PROCESS_CPUTIME_ID)
#define THREADID_TO_CLOCKID(tid) \
		((tid) * MAX_CLOCKS + CLOCK_THREAD_CPUTIME_ID)
#define CLOCKID_TO_THREADID(cid) \
		(((cid) - CLOCK_THREAD_CPUTIME_ID) / MAX_CLOCKS)
#define CLOCKID_IS_THREAD(cid) \
		(CLOCKID(cid) == CLOCK_THREAD_CPUTIME_ID)

/* Largest delay in ms for sleep and alarm calls.
   Allow actual delay to exceed requested delay by 10 s.
   Express as multiple of 1000 (i.e. seconds) + max resolution
   The tv_sec argument in timeval structures cannot exceed
   CLOCK_DELAY_MAX / 1000 - 1, so that adding fractional part
   and rounding won't exceed CLOCK_DELAY_MAX */
#define CLOCK_DELAY_MAX ((((UINT_MAX - 10000) / 1000) * 1000) + 10)

/* 100ns difference between Windows and UNIX timebase. */
#define FACTOR (0x19db1ded53e8000LL)
/* # of nanosecs per second. */
#define NSPERSEC (1000000000LL)
/* # of 100ns intervals per second. */
#define NS100PERSEC (10000000LL)
/* # of microsecs per second. */
#define USPERSEC (1000000LL)
/* # of millisecs per second. */
#define MSPERSEC (1000L)

class clk_t
{
 protected:
  /* Some values are returned as ticks/s, some as 100ns period of a
     single tick.  Store the original value and use a computation method
     making the most sense for the value given, to avoid rounding issues. */
  union
    {
      LONGLONG ticks_per_sec;
      LONGLONG period;
    };
  void init ();
  virtual int now (clockid_t, struct timespec *) = 0;

 public:
  int nsecs (clockid_t _id, struct timespec *ts)
  {
    return now (_id, ts);
  }
  virtual void resolution (struct timespec *);

  /* shortcuts for non-process/thread clocks */
  void nsecs (struct timespec *ts)
  {
    now (0, ts);
  }
  ULONGLONG nsecs ()
  {
    struct timespec ts;
    now (0, &ts);
    return (ULONGLONG) ts.tv_sec * NSPERSEC + ts.tv_nsec;
  }
  LONGLONG n100secs ()
  {
    struct timespec ts;
    now (0, &ts);
    return ts.tv_sec * NS100PERSEC + ts.tv_nsec / (NSPERSEC/NS100PERSEC);
  }
  LONGLONG usecs ()
  {
    struct timespec ts;
    now (0, &ts);
    return ts.tv_sec * USPERSEC + ts.tv_nsec / (NSPERSEC/USPERSEC);
  }
  LONGLONG msecs ()
  {
    struct timespec ts;
    now (0, &ts);
    return ts.tv_sec * MSPERSEC + ts.tv_nsec / (NSPERSEC/MSPERSEC);
  }
};

class clk_realtime_coarse_t : public clk_t
{
  virtual int now (clockid_t, struct timespec *);
};

class clk_realtime_t : public clk_t
{
  void init ();
  virtual int now (clockid_t, struct timespec *);
 public:
  virtual void resolution (struct timespec *);
};

class clk_process_t : public clk_t
{
  virtual int now (clockid_t, struct timespec *);
};

class clk_thread_t : public clk_t
{
  virtual int now (clockid_t, struct timespec *);
};

class clk_monotonic_t : public clk_t
{
 protected:
  void init ();
 private:
  virtual int now (clockid_t, struct timespec *);
 public:
  virtual void resolution (struct timespec *);
  /* Under strace 1st call is so early that vtable is NULL. */
  LONGLONG strace_usecs ()
  {
    struct timespec ts;
    clk_monotonic_t::now (0, &ts);
    return ts.tv_sec * USPERSEC + ts.tv_nsec / (NSPERSEC/USPERSEC);
  }
};

class clk_monotonic_coarse_t : public clk_t
{
  virtual int now (clockid_t, struct timespec *);
};

class clk_boottime_t : public clk_monotonic_t
{
  virtual int now (clockid_t, struct timespec *);
};

clk_t *get_clock (clockid_t clk_id);

/* Compute interval between two timespec timestamps: ts1  = ts1 - ts0. */
static inline void
ts_diff (const struct timespec &ts0, struct timespec &ts1)
{
  ts1.tv_nsec -= ts0.tv_nsec;
  if (ts1.tv_nsec < 0)
    {
      ts1.tv_nsec += NSPERSEC;
      --ts1.tv_sec;
    }
  ts1.tv_sec -= ts0.tv_sec;
}

static inline bool
valid_timespec (const timespec& ts)
{
  if (ts.tv_nsec < 0 || ts.tv_nsec >= NSPERSEC || ts.tv_sec < 0)
    return false;
  return true;
}

#endif /*__CLOCK_H__*/
