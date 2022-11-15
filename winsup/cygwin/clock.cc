#include "winsup.h"
#include <realtimeapiset.h>
#include "pinfo.h"
#include "clock.h"
#include "miscfuncs.h"
#include "spinlock.h"

static inline LONGLONG
system_qpc_tickspersec ()
{
  LARGE_INTEGER qpf;

  /* ticks per sec */
  QueryPerformanceFrequency (&qpf);
  return qpf.QuadPart;
}

static inline LONGLONG
system_tickcount_period ()
{
  ULONG coarsest = 0, finest, actual;

  if (!coarsest)
    {
      /* The actual resolution of the OS timer is a system-wide setting which
	 can be changed any time, by any process.  The only fixed value we
	 can rely on is the coarsest value. */
      NtQueryTimerResolution (&coarsest, &finest, &actual);
    }
  return coarsest;
}

void inline
clk_t::init ()
{
  if (!period)
    InterlockedExchange64 (&period, system_tickcount_period ());
}

void inline
clk_realtime_t::init ()
{
  if (!ticks_per_sec)
    InterlockedExchange64 (&ticks_per_sec, system_qpc_tickspersec ());
}

void inline
clk_monotonic_t::init ()
{
  if (!ticks_per_sec)
    InterlockedExchange64 (&ticks_per_sec, system_qpc_tickspersec ());
}

int
clk_realtime_coarse_t::now (clockid_t clockid, struct timespec *ts)
{
  LARGE_INTEGER now;

  GetSystemTimeAsFileTime ((LPFILETIME) &now);
  /* Add conversion factor for UNIX vs. Windows base time */
  now.QuadPart -= FACTOR;
  ts->tv_sec = now.QuadPart / NS100PERSEC;
  ts->tv_nsec = (now.QuadPart % NS100PERSEC) * (NSPERSEC/NS100PERSEC);
  return 0;
}

int
clk_realtime_t::now (clockid_t clockid, struct timespec *ts)
{
  LARGE_INTEGER now;

  GetSystemTimePreciseAsFileTime ((LPFILETIME) &now);
  /* Add conversion factor for UNIX vs. Windows base time */
  now.QuadPart -= FACTOR;
  ts->tv_sec = now.QuadPart / NS100PERSEC;
  ts->tv_nsec = (now.QuadPart % NS100PERSEC) * (NSPERSEC/NS100PERSEC);
  return 0;
}

int
clk_process_t::now (clockid_t clockid, struct timespec *ts)
{
  pid_t pid = CLOCKID_TO_PID (clockid);
  HANDLE hProcess;
  KERNEL_USER_TIMES kut;
  int64_t x;

  if (pid == 0)
    pid = myself->pid;

  pinfo p (pid);
  if (!p || !p->exists ())
    {
      set_errno (EINVAL);
      return -1;
    }

  hProcess = OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION, 0,
			  p->dwProcessId);
  NtQueryInformationProcess (hProcess, ProcessTimes,
			     &kut, sizeof kut, NULL);

  x = kut.KernelTime.QuadPart + kut.UserTime.QuadPart;
  ts->tv_sec = x / NS100PERSEC;
  ts->tv_nsec = (x % NS100PERSEC) * (NSPERSEC/NS100PERSEC);
  CloseHandle (hProcess);
  return 0;
}

int
clk_thread_t::now (clockid_t clockid, struct timespec *ts)
{
  long thr_id = CLOCKID_TO_THREADID (clockid);
  HANDLE hThread;
  KERNEL_USER_TIMES kut;
  int64_t x;

  if (thr_id == 0)
    thr_id = pthread::self ()->getsequence_np ();

  hThread = OpenThread (THREAD_QUERY_LIMITED_INFORMATION, 0, thr_id);
  if (!hThread)
    {
      set_errno (EINVAL);
      return -1;
    }

  NtQueryInformationThread (hThread, ThreadTimes,
			    &kut, sizeof kut, NULL);

  x = kut.KernelTime.QuadPart + kut.UserTime.QuadPart;
  ts->tv_sec = x / NS100PERSEC;
  ts->tv_nsec = (x % NS100PERSEC) * (NSPERSEC/NS100PERSEC);
  CloseHandle (hThread);
  return 0;
}

int
clk_monotonic_t::now (clockid_t clockid, struct timespec *ts)
{
  if (wincap.has_precise_interrupt_time ())
    {
      /* Suspend time not taken into account, as on Linux */
      ULONGLONG now;

      QueryUnbiasedInterruptTimePrecise (&now);
      ts->tv_sec = now / NS100PERSEC;
      now %= NS100PERSEC;
      ts->tv_nsec = now * (NSPERSEC/NS100PERSEC);
    }
  else
    {
      /* https://stackoverflow.com/questions/24330496.  Skip rounding since
         its almost always wrong when working with timestamps */
      UINT64 bias;
      LARGE_INTEGER now;
      struct timespec bts;

      init ();
      do
	{
	  bias = SharedUserData.InterruptTimeBias;
	  QueryPerformanceCounter(&now);
	}
      while (bias != SharedUserData.InterruptTimeBias);
      /* Convert perf counter to timespec */
      ts->tv_sec = now.QuadPart / ticks_per_sec;
      now.QuadPart %= ticks_per_sec;
      ts->tv_nsec = (now.QuadPart * NSPERSEC) / ticks_per_sec;
      /* Convert bias to timespec */
      bts.tv_sec = bias / NS100PERSEC;
      bias %= NS100PERSEC;
      bts.tv_nsec = bias * (NSPERSEC/NS100PERSEC);
      /* Subtract bias from perf */
      ts_diff (bts, *ts);
    }
  return 0;
}

int
clk_monotonic_coarse_t::now (clockid_t clockid, struct timespec *ts)
{
  /* Suspend time not taken into account, as on Linux */
  ULONGLONG now;

  QueryUnbiasedInterruptTime (&now);
  ts->tv_sec = now / NS100PERSEC;
  now %= NS100PERSEC;
  ts->tv_nsec = now * (NSPERSEC/NS100PERSEC);
  return 0;
}

int
clk_boottime_t::now (clockid_t clockid, struct timespec *ts)
{
  /* Suspend time taken into account, as on Linux */
  if (wincap.has_precise_interrupt_time ())
    {
      ULONGLONG now;

      QueryInterruptTimePrecise (&now);
      ts->tv_sec = now / NS100PERSEC;
      now %= NS100PERSEC;
      ts->tv_nsec = now * (NSPERSEC/NS100PERSEC);
    }
  else
    {
      LARGE_INTEGER now;

      init ();
      QueryPerformanceCounter (&now);
      ts->tv_sec = now.QuadPart / ticks_per_sec;
      now.QuadPart %= ticks_per_sec;
      ts->tv_nsec = (now.QuadPart * NSPERSEC) / ticks_per_sec;
    }
  return 0;
}

void
clk_t::resolution (struct timespec *ts)
{
  init ();
  ts->tv_sec = 0;
  ts->tv_nsec = period * (NSPERSEC/NS100PERSEC);
}

void
clk_realtime_t::resolution (struct timespec *ts)
{
  init ();
  ts->tv_sec = 0;
  ts->tv_nsec = NSPERSEC / ticks_per_sec;
}

void
clk_monotonic_t::resolution (struct timespec *ts)
{
  init ();
  ts->tv_sec = 0;
  ts->tv_nsec = NSPERSEC / ticks_per_sec;
}

static clk_realtime_coarse_t clk_realtime_coarse;
static clk_realtime_t clk_realtime;
static clk_process_t clk_process;
static clk_thread_t clk_thread;
static clk_monotonic_t clk_monotonic;
static clk_monotonic_t clk_monotonic_raw;	/* same as clk_monotonic */
static clk_monotonic_coarse_t clk_monotonic_coarse;
static clk_boottime_t clk_boottime;
static clk_realtime_t clk_realtime_alarm;	/* same as clk_realtime */
static clk_boottime_t clk_boottime_alarm;	/* same as clk_boottime_t */

clk_t *cyg_clock[MAX_CLOCKS] =
{
  &clk_realtime_coarse,
  &clk_realtime,
  &clk_process,
  &clk_thread,
  &clk_monotonic,
  &clk_monotonic_raw,
  &clk_monotonic_coarse,
  &clk_boottime,
  &clk_realtime_alarm,
  &clk_boottime_alarm,
};

clk_t *
get_clock (clockid_t clk_id)
{
  extern clk_t *cyg_clock[MAX_CLOCKS];
  clockid_t clockid = CLOCKID (clk_id);
  if (clk_id >= MAX_CLOCKS
      && clockid != CLOCK_PROCESS_CPUTIME_ID
      && clockid != CLOCK_THREAD_CPUTIME_ID)
    return NULL;
  return cyg_clock[clockid];
}
