/* times.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009, 2010, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define __timezonefunc__
#include "winsup.h"
#include <sys/times.h>
#include <sys/timeb.h>
#include <utime.h>
#include <stdlib.h>
#include <unistd.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "thread.h"
#include "cygtls.h"
#include "ntdll.h"

/* Max allowed diversion in 100ns of internal timer from system time.  If
   this difference is exceeded, the internal timer gets re-primed. */
#define JITTER (40 * 10000LL)

/* TODO: Putting this variable in the shared cygwin region partially solves
   the problem of cygwin processes not recognizing date changes when other
   cygwin processes set the date.  There is still an additional problem of
   long-running cygwin processes becoming confused when a non-cygwin process
   sets the date.  Unfortunately, it looks like a minor redesign is required
   to handle that case.  */
hires_ms gtod __attribute__((section (".cygwin_dll_common"), shared));

hires_ns NO_COPY ntod;

static inline LONGLONG
systime_ns ()
{
  LARGE_INTEGER x;
  GetSystemTimeAsFileTime ((LPFILETIME) &x);
  x.QuadPart -= FACTOR;		/* Add conversion factor for UNIX vs. Windows base time */
  return x.QuadPart;
}

/* Cygwin internal */
static unsigned long long __stdcall
__to_clock_t (FILETIME *src, int flag)
{
  unsigned long long total = ((unsigned long long) src->dwHighDateTime << 32) + ((unsigned)src->dwLowDateTime);
  syscall_printf ("dwHighDateTime %u, dwLowDateTime %u", src->dwHighDateTime, src->dwLowDateTime);

  /* Convert into clock ticks - the total is in 10ths of a usec.  */
  if (flag)
    total -= FACTOR;

  total /= (unsigned long long) (NSPERSEC / CLOCKS_PER_SEC);
  syscall_printf ("total %08x %08x", (unsigned) (total>>32), (unsigned) (total));
  return total;
}

/* times: POSIX 4.5.2.1 */
extern "C" clock_t
times (struct tms *buf)
{
  FILETIME creation_time, exit_time, kernel_time, user_time;

  myfault efault;
  if (efault.faulted (EFAULT))
    return ((clock_t) -1);

  LONGLONG ticks = gtod.uptime ();
  /* Ticks is in milliseconds, convert to our ticks. Use long long to prevent
     overflow. */
  clock_t tc = (clock_t) (ticks * CLOCKS_PER_SEC / 1000);

  GetProcessTimes (GetCurrentProcess (), &creation_time, &exit_time,
		   &kernel_time, &user_time);

  syscall_printf ("ticks %d, CLOCKS_PER_SEC %d", ticks, CLOCKS_PER_SEC);
  syscall_printf ("user_time %d, kernel_time %d, creation_time %d, exit_time %d",
		  user_time, kernel_time, creation_time, exit_time);
  buf->tms_stime = __to_clock_t (&kernel_time, 0);
  buf->tms_utime = __to_clock_t (&user_time, 0);
  timeval_to_filetime (&myself->rusage_children.ru_stime, &kernel_time);
  buf->tms_cstime = __to_clock_t (&kernel_time, 1);
  timeval_to_filetime (&myself->rusage_children.ru_utime, &user_time);
  buf->tms_cutime = __to_clock_t (&user_time, 1);

  return tc;
}

EXPORT_ALIAS (times, _times)

/* settimeofday: BSD */
extern "C" int
settimeofday (const struct timeval *tv, const struct timezone *tz)
{
  SYSTEMTIME st;
  struct tm *ptm;
  int res;

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  if (tv->tv_usec < 0 || tv->tv_usec >= 1000000)
    {
      set_errno (EINVAL);
      return -1;
    }

  ptm = gmtime (&tv->tv_sec);
  st.wYear	   = ptm->tm_year + 1900;
  st.wMonth	   = ptm->tm_mon + 1;
  st.wDayOfWeek    = ptm->tm_wday;
  st.wDay	   = ptm->tm_mday;
  st.wHour	   = ptm->tm_hour;
  st.wMinute       = ptm->tm_min;
  st.wSecond       = ptm->tm_sec;
  st.wMilliseconds = tv->tv_usec / 1000;

  res = -!SetSystemTime (&st);
  gtod.reset ();

  if (res)
    set_errno (EPERM);

  syscall_printf ("%R = settimeofday(%x, %x)", res, tv, tz);
  return res;
}

/* timezone: standards? */
extern "C" char *
timezone (void)
{
  char *b = _my_tls.locals.timezone_buf;

  tzset ();
  __small_sprintf (b,"GMT%+d:%02d", (int) (-_timezone / 3600), (int) (abs (_timezone / 60) % 60));
  return b;
}

/* Cygwin internal */
void __stdcall
totimeval (struct timeval *dst, FILETIME *src, int sub, int flag)
{
  long long x = __to_clock_t (src, flag);

  x *= (int) (1e6) / CLOCKS_PER_SEC; /* Turn x into usecs */
  x -= (long long) sub * (int) (1e6);

  dst->tv_usec = x % (long long) (1e6); /* And split */
  dst->tv_sec = x / (long long) (1e6);
}

/* FIXME: Make thread safe */
extern "C" int
gettimeofday (struct timeval *tv, void *tzvp)
{
  struct timezone *tz = (struct timezone *) tzvp;
  static bool tzflag;
  LONGLONG now = gtod.usecs ();

  if (now == (LONGLONG) -1)
    return -1;

  tv->tv_sec = now / 1000000;
  tv->tv_usec = now % 1000000;

  if (tz != NULL)
    {
      if (!tzflag)
	{
	  tzset ();
	  tzflag = true;
	}
      tz->tz_minuteswest = _timezone / 60;
      tz->tz_dsttime = _daylight;
    }

  return 0;
}

EXPORT_ALIAS (gettimeofday, _gettimeofday)

/* Cygwin internal */
void
time_t_to_filetime (time_t time_in, FILETIME *out)
{
  long long x = time_in * NSPERSEC + FACTOR;
  out->dwHighDateTime = x >> 32;
  out->dwLowDateTime = x;
}

/* Cygwin internal */
void __stdcall
timespec_to_filetime (const struct timespec *time_in, FILETIME *out)
{
  if (time_in->tv_nsec == UTIME_OMIT)
    out->dwHighDateTime = out->dwLowDateTime = 0;
  else
    {
      long long x = time_in->tv_sec * NSPERSEC +
			    time_in->tv_nsec / (1000000000/NSPERSEC) + FACTOR;
      out->dwHighDateTime = x >> 32;
      out->dwLowDateTime = x;
    }
}

/* Cygwin internal */
void __stdcall
timeval_to_filetime (const struct timeval *time_in, FILETIME *out)
{
  long long x = time_in->tv_sec * NSPERSEC +
			time_in->tv_usec * (NSPERSEC/1000000) + FACTOR;
  out->dwHighDateTime = x >> 32;
  out->dwLowDateTime = x;
}

/* Cygwin internal */
static timeval __stdcall
time_t_to_timeval (time_t in)
{
  timeval res;
  res.tv_sec = in;
  res.tv_usec = 0;
  return res;
}

/* Cygwin internal */
static const struct timespec *
timeval_to_timespec (const struct timeval *tvp, struct timespec *tmp)
{
  if (!tvp)
    return NULL;

  tmp[0].tv_sec = tvp[0].tv_sec;
  tmp[0].tv_nsec = tvp[0].tv_usec * 1000;
  if (tmp[0].tv_nsec < 0)
    tmp[0].tv_nsec = 0;
  else if (tmp[0].tv_nsec > 999999999)
    tmp[0].tv_nsec = 999999999;

  tmp[1].tv_sec = tvp[1].tv_sec;
  tmp[1].tv_nsec = tvp[1].tv_usec * 1000;
  if (tmp[1].tv_nsec < 0)
    tmp[1].tv_nsec = 0;
  else if (tmp[1].tv_nsec > 999999999)
    tmp[1].tv_nsec = 999999999;

  return tmp;
}

/* Cygwin internal */
/* Convert a Win32 time to "UNIX" format. */
long __stdcall
to_time_t (FILETIME *ptr)
{
  /* A file time is the number of 100ns since jan 1 1601
     stuffed into two long words.
     A time_t is the number of seconds since jan 1 1970.  */

  long long x = ((long long) ptr->dwHighDateTime << 32) + ((unsigned)ptr->dwLowDateTime);

  /* pass "no time" as epoch */
  if (x == 0)
    return 0;

  x -= FACTOR;			/* number of 100ns between 1601 and 1970 */
  x /= (long long) NSPERSEC;		/* number of 100ns in a second */
  return x;
}

/* Cygwin internal */
/* Convert a Win32 time to "UNIX" timestruc_t format. */
void __stdcall
to_timestruc_t (FILETIME *ptr, timestruc_t *out)
{
  /* A file time is the number of 100ns since jan 1 1601
     stuffed into two long words.
     A timestruc_t is the number of seconds and microseconds since jan 1 1970
     stuffed into a time_t and a long.  */

  long rem;
  long long x = ((long long) ptr->dwHighDateTime << 32) + ((unsigned)ptr->dwLowDateTime);

  /* pass "no time" as epoch */
  if (x == 0)
    {
      out->tv_sec = 0;
      out->tv_nsec = 0;
      return;
    }

  x -= FACTOR;			/* number of 100ns between 1601 and 1970 */
  rem = x % ((long long)NSPERSEC);
  x /= (long long) NSPERSEC;		/* number of 100ns in a second */
  out->tv_nsec = rem * 100;	/* as tv_nsec is in nanoseconds */
  out->tv_sec = x;
}

/* Cygwin internal */
/* Get the current time as a "UNIX" timestruc_t format. */
void __stdcall
time_as_timestruc_t (timestruc_t * out)
{
  FILETIME filetime;

  GetSystemTimeAsFileTime (&filetime);
  to_timestruc_t (&filetime, out);
}

/* time: POSIX 4.5.1.1, C 4.12.2.4 */
/* Return number of seconds since 00:00 UTC on jan 1, 1970 */
extern "C" time_t
time (time_t * ptr)
{
  time_t res;
  FILETIME filetime;

  GetSystemTimeAsFileTime (&filetime);
  res = to_time_t (&filetime);
  if (ptr)
    *ptr = res;

  syscall_printf ("%d = time(%x)", res, ptr);

  return res;
}

int
utimens_worker (path_conv &win32, const struct timespec *tvp)
{
  int res = -1;

  if (win32.error)
    set_errno (win32.error);
  else
    {
      fhandler_base *fh = NULL;
      bool fromfd = false;

      cygheap_fdenum cfd (true);
      while (cfd.next () >= 0)
	if (cfd->get_access () & (FILE_WRITE_ATTRIBUTES | GENERIC_WRITE)
	    && RtlEqualUnicodeString (cfd->pc.get_nt_native_path (),
				      win32.get_nt_native_path (),
				      cfd->pc.objcaseinsensitive ()))
	  {
	    fh = cfd;
	    fromfd = true;
	    break;
	  }

      if (!fh)
	{
	  if (!(fh = build_fh_pc (win32)))
	    goto error;

	  if (fh->error ())
	    {
	      debug_printf ("got %d error from build_fh_pc", fh->error ());
	      set_errno (fh->error ());
	    }
	}

      res = fh->utimens (tvp);

      if (!fromfd)
	delete fh;
    }

error:
  syscall_printf ("%R = utimes(%S, %p)", res, win32.get_nt_native_path (), tvp);
  return res;
}

/* utimes: POSIX/SUSv3 */
extern "C" int
utimes (const char *path, const struct timeval *tvp)
{
  path_conv win32 (path, PC_POSIX | PC_SYM_FOLLOW, stat_suffixes);
  struct timespec tmp[2];
  return utimens_worker (win32, timeval_to_timespec (tvp, tmp));
}

/* BSD */
extern "C" int
lutimes (const char *path, const struct timeval *tvp)
{
  path_conv win32 (path, PC_POSIX | PC_SYM_NOFOLLOW, stat_suffixes);
  struct timespec tmp[2];
  return utimens_worker (win32, timeval_to_timespec (tvp, tmp));
}

/* futimens: POSIX/SUSv4 */
extern "C" int
futimens (int fd, const struct timespec *tvp)
{
  int res;

  cygheap_fdget cfd (fd);
  if (cfd < 0)
    res = -1;
  else if (cfd->get_access () & (FILE_WRITE_ATTRIBUTES | GENERIC_WRITE))
    res = cfd->utimens (tvp);
  else
    res = utimens_worker (cfd->pc, tvp);
  syscall_printf ("%d = futimens(%d, %p)", res, fd, tvp);
  return res;
}

/* BSD */
extern "C" int
futimes (int fd, const struct timeval *tvp)
{
  struct timespec tmp[2];
  return futimens (fd,  timeval_to_timespec (tvp, tmp));
}

/* utime: POSIX 5.6.6.1 */
extern "C" int
utime (const char *path, const struct utimbuf *buf)
{
  struct timeval tmp[2];

  if (buf == 0)
    return utimes (path, 0);

  debug_printf ("incoming utime act %x", buf->actime);
  tmp[0] = time_t_to_timeval (buf->actime);
  tmp[1] = time_t_to_timeval (buf->modtime);

  return utimes (path, tmp);
}

/* ftime: standards? */
extern "C" int
ftime (struct timeb *tp)
{
  struct timeval tv;
  struct timezone tz;

  if (gettimeofday (&tv, &tz) < 0)
    return -1;

  tp->time = tv.tv_sec;
  tp->millitm = tv.tv_usec / 1000;
  tp->timezone = tz.tz_minuteswest;
  tp->dstflag = tz.tz_dsttime;

  return 0;
}

#define stupid_printf if (cygwin_finished_initializing) debug_printf
void
hires_ns::prime ()
{
  LARGE_INTEGER ifreq;
  if (!QueryPerformanceFrequency (&ifreq))
    {
      inited = -1;
      return;
    }

  int priority = GetThreadPriority (GetCurrentThread ());

  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);
  if (!QueryPerformanceCounter (&primed_pc))
    {
      SetThreadPriority (GetCurrentThread (), priority);
      inited = -1;
      return;
    }

  freq = (double) ((double) 1000000000. / (double) ifreq.QuadPart);
  inited = true;
  SetThreadPriority (GetCurrentThread (), priority);
}

LONGLONG
hires_ns::nsecs ()
{
  if (!inited)
    prime ();
  if (inited < 0)
    {
      set_errno (ENOSYS);
      return (long long) -1;
    }

  LARGE_INTEGER now;
  if (!QueryPerformanceCounter (&now))
    {
      set_errno (ENOSYS);
      return -1;
    }

  // FIXME: Use round() here?
  now.QuadPart = (LONGLONG) (freq * (double) (now.QuadPart - primed_pc.QuadPart));
  return now.QuadPart;
}

LONGLONG
hires_ms::timeGetTime_ns ()
{
  LARGE_INTEGER t;

  /* This is how timeGetTime is implemented in winmm.dll.
     The real timeGetTime subtracts and adds some values which are constant
     over the lifetime of the process.  Since we don't need absolute accuracy
     of the value returned by timeGetTime, only relative accuracy, we can skip
     this step.  However, if we ever find out that we need absolute accuracy,
     here's how it works in it's full beauty:

     - At process startup, winmm initializes two calibration values:

       DWORD tick_count_start;
       LARGE_INTEGER int_time_start;
       do
	 {
	   tick_count_start = GetTickCount ();
	   do
	     {
	       int_time_start.HighPart = SharedUserData.InterruptTime.High1Time;
	       int_time_start.LowPart = SharedUserData.InterruptTime.LowPart;
	     }
	   while (int_time_start.HighPart
		  != SharedUserData.InterruptTime.High2Time);
	 }
       while (tick_count_start != GetTickCount ();

     - timeGetTime computes its return value in the loop as below, and then:

       t.QuadPart -= int_time_start.QuadPart;
       t.QuadPart /= 10000;
       t.LowPart += tick_count_start;
       return t.LowPart;
  */
  do
    {
      t.HighPart = SharedUserData.InterruptTime.High1Time;
      t.LowPart = SharedUserData.InterruptTime.LowPart;
    }
  while (t.HighPart != SharedUserData.InterruptTime.High2Time);
  /* We use the value in full 100ns resolution in the calling functions
     anyway, so we can skip dividing by 10000 here. */
  return t.QuadPart;
}

void
hires_ms::prime ()
{
  if (!inited)
    {
      int priority = GetThreadPriority (GetCurrentThread ());
      SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);
      initime_ns = systime_ns () - timeGetTime_ns ();
      inited = true;
      SetThreadPriority (GetCurrentThread (), priority);
    }
  return;
}

LONGLONG
hires_ms::nsecs ()
{
  if (!inited)
    prime ();

  LONGLONG t = systime_ns ();
  LONGLONG res = initime_ns + timeGetTime_ns ();
  if (llabs (res - t) > JITTER)
    {
      inited = false;
      prime ();
      res = initime_ns + timeGetTime_ns ();
    }
  return res;
}

extern "C" int
clock_gettime (clockid_t clk_id, struct timespec *tp)
{
  if (CLOCKID_IS_PROCESS (clk_id))
    {
      pid_t pid = CLOCKID_TO_PID (clk_id);
      HANDLE hProcess;
      KERNEL_USER_TIMES kut;
      ULONG sizeof_kut = sizeof (KERNEL_USER_TIMES);
      long long x;

      if (pid == 0)
	pid = getpid ();

      pinfo p (pid);
      if (!p->exists ())
	{
	  set_errno (EINVAL);
	  return -1;
	}

      hProcess = OpenProcess (PROCESS_QUERY_INFORMATION, 0, p->dwProcessId);
      NtQueryInformationProcess (hProcess, ProcessTimes, &kut, sizeof_kut, &sizeof_kut);

      x = kut.KernelTime.QuadPart + kut.UserTime.QuadPart;
      tp->tv_sec = x / (long long) NSPERSEC;
      tp->tv_nsec = (x % (long long) NSPERSEC) * 100LL;

      CloseHandle (hProcess);
      return 0;
    }

  if (CLOCKID_IS_THREAD (clk_id))
    {
      long thr_id = CLOCKID_TO_THREADID (clk_id);
      HANDLE hThread;
      KERNEL_USER_TIMES kut;
      ULONG sizeof_kut = sizeof (KERNEL_USER_TIMES);
      long long x;

      if (thr_id == 0)
	thr_id = pthread::self ()->getsequence_np ();

      hThread = OpenThread (THREAD_QUERY_INFORMATION, 0, thr_id);
      if (!hThread)
	{
	  set_errno (EINVAL);
	  return -1;
	}

      NtQueryInformationThread (hThread, ThreadTimes, &kut, sizeof_kut, &sizeof_kut);

      x = kut.KernelTime.QuadPart + kut.UserTime.QuadPart;
      tp->tv_sec = x / (long long) NSPERSEC;
      tp->tv_nsec = (x % (long long) NSPERSEC) * 100LL;

      CloseHandle (hThread);
      return 0;
    }

  switch (clk_id)
    {
      case CLOCK_REALTIME:
	{
	  LONGLONG now = gtod.nsecs ();
	  if (now == (LONGLONG) -1)
	    return -1;
	  tp->tv_sec = now / NSPERSEC;
	  tp->tv_nsec = (now % NSPERSEC) * (1000000000 / NSPERSEC);
	  break;
	}

      case CLOCK_MONOTONIC:
	{
	  LONGLONG now = ntod.nsecs ();
	  if (now == (LONGLONG) -1)
	    return -1;

	  tp->tv_sec = now / 1000000000;
	  tp->tv_nsec = (now % 1000000000);
	  break;
	}

      default:
	set_errno (EINVAL);
	return -1;
    }

  return 0;
}

extern "C" int
clock_settime (clockid_t clk_id, const struct timespec *tp)
{
  struct timeval tv;

  if (CLOCKID_IS_PROCESS (clk_id) || CLOCKID_IS_THREAD (clk_id))
    /* According to POSIX, the privileges to set a particular clock
     * are implementation-defined.  On Linux, CPU-time clocks are not
     * settable; do the same here.
     */
    {
      set_errno (EPERM);
      return -1;
    }

  if (clk_id != CLOCK_REALTIME)
    {
      set_errno (EINVAL);
      return -1;
    }

  tv.tv_sec = tp->tv_sec;
  tv.tv_usec = tp->tv_nsec / 1000;

  return settimeofday (&tv, NULL);
}

static DWORD minperiod;	// FIXME: Maintain period after a fork.

LONGLONG
hires_ns::resolution ()
{
  if (!inited)
    prime ();
  if (inited < 0)
    {
      set_errno (ENOSYS);
      return (long long) -1;
    }

  return (freq <= 1.0) ? 1LL : (LONGLONG) freq;
}

UINT
hires_ms::resolution ()
{
  if (!minperiod)
    {
      NTSTATUS status;
      ULONG coarsest, finest, actual;

      status = NtQueryTimerResolution (&coarsest, &finest, &actual);
      if (NT_SUCCESS (status))
	minperiod = (DWORD) actual;
      else
	{
	  /* Try to empirically determine current timer resolution */
	  int priority = GetThreadPriority (GetCurrentThread ());
	  SetThreadPriority (GetCurrentThread (),
			     THREAD_PRIORITY_TIME_CRITICAL);
	  LONGLONG period = 0;
	  for (int i = 0; i < 4; i++)
	    {
	      LONGLONG now;
	      LONGLONG then = timeGetTime_ns ();
	      while ((now = timeGetTime_ns ()) == then)
		continue;
	      then = now;
	      while ((now = timeGetTime_ns ()) == then)
		continue;
	      period += now - then;
	    }
	  SetThreadPriority (GetCurrentThread (), priority);
	  period /= 4L;
	  minperiod = (DWORD) period;
	}
    }
  return minperiod;
}

extern "C" int
clock_getres (clockid_t clk_id, struct timespec *tp)
{
  if (CLOCKID_IS_PROCESS (clk_id) || CLOCKID_IS_THREAD (clk_id))
    {
      ULONG coarsest, finest, actual;

      NtQueryTimerResolution (&coarsest, &finest, &actual);
      tp->tv_sec = coarsest / NSPERSEC;
      tp->tv_nsec = (coarsest % NSPERSEC) * 100;
      return 0;
    }

  switch (clk_id)
    {
      case CLOCK_REALTIME:
	{
	  DWORD period = gtod.resolution ();
	  tp->tv_sec = period / NSPERSEC;
	  tp->tv_nsec = (period % NSPERSEC) * 100;
	  break;
	}

      case CLOCK_MONOTONIC:
	{
	  LONGLONG period = ntod.resolution ();
	  tp->tv_sec = period / 1000000000;
	  tp->tv_nsec = period % 1000000000;
	  break;
	}

      default:
	set_errno (EINVAL);
	return -1;
    }

  return 0;
}

extern "C" int
clock_setres (clockid_t clk_id, struct timespec *tp)
{
  static NO_COPY bool period_set;
  int status;

  if (clk_id != CLOCK_REALTIME)
    {
      set_errno (EINVAL);
      return -1;
    }

  /* Convert to 100ns to match OS resolution.  The OS uses ULONG values
     to express resolution in 100ns units, so the coarsest timer resolution
     is < 430 secs.  Actually the coarsest timer resolution is only slightly
     beyond 15ms, but this might change in future OS versions, so we play nice
     here. */
  ULONGLONG period = (tp->tv_sec * 10000000ULL) + ((tp->tv_nsec) / 100ULL);

  /* clock_setres is non-POSIX/non-Linux.  On QNX, the function always
     rounds the incoming value to the nearest supported value. */
  ULONG coarsest, finest, actual;
  if (NT_SUCCESS (NtQueryTimerResolution (&coarsest, &finest, &actual)))
    {
      if (period > coarsest)
	period = coarsest;
      else if (finest > period)
	period = finest;
    }

  if (period_set
      && NT_SUCCESS (NtSetTimerResolution (minperiod, FALSE, &actual)))
    period_set = false;

  status = NtSetTimerResolution (period, TRUE, &actual);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  minperiod = actual;
  period_set = true;
  return 0;
}

extern "C" int
clock_getcpuclockid (pid_t pid, clockid_t *clk_id)
{
  if (pid != 0 && !pinfo (pid)->exists ())
    return (ESRCH);
  *clk_id = (clockid_t) PID_TO_CLOCKID (pid);
  return 0;
}
