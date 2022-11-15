/* times.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define __timezonefunc__
#include "winsup.h"
#include <sys/times.h>
#include <sys/timeb.h>
#include <sys/param.h>
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
#include "spinlock.h"

static inline void __attribute__ ((always_inline))
get_system_time (PLARGE_INTEGER systime)
{
  GetSystemTimePreciseAsFileTime ((LPFILETIME) systime);
}

/* Cygwin internal */
static uint64_t
__to_clock_t (PLARGE_INTEGER src, int flag)
{
  uint64_t total = src->QuadPart;
  /* Convert into clock ticks - the total is in 10ths of a usec.  */
  if (flag)
    total -= FACTOR;

  total /= NS100PERSEC / CLOCKS_PER_SEC;
  return total;
}

/* times: POSIX 4.5.2.1 */
extern "C" clock_t
times (struct tms *buf)
{
  static SYSTEM_TIMEOFDAY_INFORMATION stodi;
  KERNEL_USER_TIMES kut;
  LARGE_INTEGER ticks;
  clock_t tc = (clock_t) -1;

  __try
    {
      /* Fetch boot time if we haven't already. */
      if (!stodi.BootTime.QuadPart)
	NtQuerySystemInformation (SystemTimeOfDayInformation,
				  &stodi, sizeof stodi, NULL);

      NtQueryInformationProcess (NtCurrentProcess (), ProcessTimes,
				 &kut, sizeof kut, NULL);
      get_system_time (&ticks);

      /* uptime */
      ticks.QuadPart -= stodi.BootTime.QuadPart;
      /* ticks is in in 100ns, convert to clock ticks. */
      tc = (clock_t) (ticks.QuadPart * CLOCKS_PER_SEC / NS100PERSEC);

      /* Linux allows a NULL buf and just returns tc in that case, so
	 mimic that */
      if (buf)
	{
	  buf->tms_stime = __to_clock_t (&kut.KernelTime, 0);
	  buf->tms_utime = __to_clock_t (&kut.UserTime, 0);
	  timeval_to_filetime (&myself->rusage_children.ru_stime, &kut.KernelTime);
	  buf->tms_cstime = __to_clock_t (&kut.KernelTime, 1);
	  timeval_to_filetime (&myself->rusage_children.ru_utime, &kut.UserTime);
	  buf->tms_cutime = __to_clock_t (&kut.UserTime, 1);
	}
    }
  __except (EFAULT)
    {
      tc = (clock_t) -1;
    }
  __endtry
  syscall_printf ("%D = times(%p)", tc, buf);
  return tc;
}

EXPORT_ALIAS (times, _times)

/* settimeofday: BSD */
extern "C" int
settimeofday (const struct timeval *tv, const struct timezone *tz)
{
  SYSTEMTIME st;
  struct tm *ptm;
  int res = -1;

  __try
    {
      if (tv->tv_usec < 0 || tv->tv_usec >= USPERSEC)
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
      st.wMilliseconds = tv->tv_usec / (USPERSEC / MSPERSEC);

      res = -!SetSystemTime (&st);
      if (res)
	set_errno (EPERM);
    }
  __except (EFAULT)
    {
      res = -1;
    }
  __endtry
  syscall_printf ("%R = settimeofday(%p, %p)", res, tv, tz);
  return res;
}

/* stime: SVr4 */
extern "C" int
stime (const time_t *t)
{
  struct timeval tv = { *t, 0 };
  return settimeofday(&tv, NULL);
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
void
totimeval (struct timeval *dst, PLARGE_INTEGER src, int sub, int flag)
{
  int64_t x = __to_clock_t (src, flag);

  x *= (int64_t) USPERSEC / CLOCKS_PER_SEC; /* Turn x into usecs */
  x -= (int64_t) sub * USPERSEC;

  dst->tv_usec = x % USPERSEC; /* And split */
  dst->tv_sec = x / USPERSEC;
}

/* FIXME: Make thread safe */
extern "C" int
gettimeofday (struct timeval *__restrict tv, void *__restrict tzvp)
{
  struct timezone *tz = (struct timezone *) tzvp;
  static bool tzflag;
  LONGLONG now = get_clock (CLOCK_REALTIME)->usecs ();

  tv->tv_sec = now / USPERSEC;
  tv->tv_usec = now % USPERSEC;

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
timespec_to_filetime (const struct timespec *time_in, PLARGE_INTEGER out)
{
  if (time_in->tv_nsec == UTIME_OMIT)
    out->QuadPart = 0;
  else
    out->QuadPart = time_in->tv_sec * NS100PERSEC
		    + time_in->tv_nsec / (NSPERSEC/NS100PERSEC) + FACTOR;
}

/* Cygwin internal */
void
timeval_to_filetime (const struct timeval *time_in, PLARGE_INTEGER out)
{
  out->QuadPart = time_in->tv_sec * NS100PERSEC
		  + time_in->tv_usec * (NS100PERSEC/USPERSEC) + FACTOR;
}

/* Cygwin internal */
bool
timeval_to_ms (const struct timeval *time_in, DWORD &ms)
{
  if (time_in->tv_sec < 0 || time_in->tv_usec < 0
      || time_in->tv_usec >= USPERSEC)
    return false;
  if ((time_in->tv_sec == 0 && time_in->tv_usec == 0)
      || time_in->tv_sec >= (time_t) (INFINITE / MSPERSEC))
    ms = INFINITE;
  else
    ms = time_in->tv_sec * MSPERSEC
	 + (time_in->tv_usec + (USPERSEC / MSPERSEC) - 1)
	   / (USPERSEC / MSPERSEC);
  return true;
}

/* Cygwin internal */
static timeval
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
  tmp[0].tv_nsec = tvp[0].tv_usec * (NSPERSEC/USPERSEC);
  if (tmp[0].tv_nsec < 0)
    tmp[0].tv_nsec = 0;
  else if (tmp[0].tv_nsec >= NSPERSEC)
    tmp[0].tv_nsec = NSPERSEC - 1;

  tmp[1].tv_sec = tvp[1].tv_sec;
  tmp[1].tv_nsec = tvp[1].tv_usec * (NSPERSEC/USPERSEC);
  if (tmp[1].tv_nsec < 0)
    tmp[1].tv_nsec = 0;
  else if (tmp[1].tv_nsec >= NSPERSEC)
    tmp[1].tv_nsec = NSPERSEC - 1;

  return tmp;
}

/* Cygwin internal */
/* Convert a Win32 time to "UNIX" format. */
time_t
to_time_t (PLARGE_INTEGER ptr)
{
  /* A file time is the number of 100ns since jan 1 1601
     stuffed into two long words.
     A time_t is the number of seconds since jan 1 1970.  */

  int64_t x = ptr->QuadPart;

  /* pass "no time" as epoch */
  if (x == 0)
    return 0;

  x -= FACTOR;			/* number of 100ns between 1601 and 1970 */
  x /= NS100PERSEC;		/* number of 100ns in a second */
  return x;
}

/* Cygwin internal */
/* Convert a Win32 time to "UNIX" timestruc_t format. */
void
to_timestruc_t (PLARGE_INTEGER ptr, timestruc_t *out)
{
  /* A file time is the number of 100ns since jan 1 1601
     stuffed into two long words.
     A timestruc_t is the number of seconds and microseconds since jan 1 1970
     stuffed into a time_t and a long.  */
  int64_t x = ptr->QuadPart;

  /* pass "no time" as epoch */
  if (x == 0)
    {
      out->tv_sec = 0;
      out->tv_nsec = 0;
      return;
    }

  x -= FACTOR;			/* number of 100ns between 1601 and 1970 */
  out->tv_sec = x / NS100PERSEC;
  out->tv_nsec = (x % NS100PERSEC) * (NSPERSEC/NS100PERSEC);
}

/* Cygwin internal */
/* Get the current time as a "UNIX" timestruc_t format. */
void
time_as_timestruc_t (timestruc_t * out)
{
  LARGE_INTEGER systime;

  get_system_time (&systime);
  to_timestruc_t (&systime, out);
}

/* time: POSIX 4.5.1.1, C 4.12.2.4 */
/* Return number of seconds since 00:00 UTC on jan 1, 1970 */
extern "C" time_t
time (time_t * ptr)
{
  time_t res;
  LARGE_INTEGER systime;

  get_system_time (&systime);
  res = to_time_t (&systime);
  if (ptr)
    *ptr = res;

  syscall_printf ("%d = time(%p)", res, ptr);

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
utimes (const char *path, const struct timeval tvp[2])
{
  path_conv win32 (path, PC_POSIX | PC_SYM_FOLLOW, stat_suffixes);
  struct timespec tmp[2];
  return utimens_worker (win32, timeval_to_timespec (tvp, tmp));
}

/* BSD */
extern "C" int
lutimes (const char *path, const struct timeval tvp[2])
{
  path_conv win32 (path, PC_POSIX | PC_SYM_NOFOLLOW, stat_suffixes);
  struct timespec tmp[2];
  return utimens_worker (win32, timeval_to_timespec (tvp, tmp));
}

/* futimens: POSIX/SUSv4 */
extern "C" int
futimens (int fd, const struct timespec tvp[2])
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
futimes (int fd, const struct timeval tvp[2])
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

  debug_printf ("incoming utime act %lx", buf->actime);
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

extern "C" int
clock_gettime (clockid_t clk_id, struct timespec *tp)
{
  clk_t *clock = get_clock (clk_id);

  if (!clock)
    {
      set_errno (EINVAL);
      return -1;
    }
  __try
    {
      return clock->nsecs (clk_id, tp);
    }
  __except (EFAULT) {}
  __endtry
  return -1;
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

  if (clk_id != CLOCK_REALTIME_COARSE && clk_id != CLOCK_REALTIME)
    {
      set_errno (EINVAL);
      return -1;
    }

  __try
    {
      tv.tv_sec = tp->tv_sec;
      tv.tv_usec = tp->tv_nsec / 1000;
    }
  __except (EFAULT)
    {
      return -1;
    }
  __endtry

  return settimeofday (&tv, NULL);
}

extern "C" int
clock_getres (clockid_t clk_id, struct timespec *tp)
{
  clk_t *clock = get_clock (clk_id);

  if (!clock)
    {
      set_errno (EINVAL);
      return -1;
    }
  __try
    {
      clock->resolution (tp);
    }
  __except (EFAULT)
    {
      return -1;
    }
  __endtry
  return 0;
}

extern "C" int
clock_setres (clockid_t clk_id, struct timespec *tp)
{
  /* Don't use this function.  It only exists in QNX.  Just return
     success on CLOCK_REALTIME for backward compat. */
  if (clk_id != CLOCK_REALTIME)
    {
      set_errno (EINVAL);
      return -1;
    }
  return 0;
}

extern "C" int
clock_getcpuclockid (pid_t pid, clockid_t *clk_id)
{
  if (pid != 0)
    {
      pinfo p (pid);
      if (!p || !p->exists ())
	return (ESRCH);
    }
  *clk_id = (clockid_t) PID_TO_CLOCKID (pid);
  return 0;
}
