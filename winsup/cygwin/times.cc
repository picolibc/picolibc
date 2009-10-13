/* times.cc

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009 Red Hat, Inc.

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
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "cygtls.h"
#include "ntdll.h"

#define FACTOR (0x19db1ded53e8000LL)
#define NSPERSEC 10000000LL

static inline LONGLONG
systime_ns ()
{
  LARGE_INTEGER x;
  FILETIME ft;
  GetSystemTimeAsFileTime (&ft);
  x.HighPart = ft.dwHighDateTime;
  x.LowPart = ft.dwLowDateTime;
  x.QuadPart -= FACTOR;		/* Add conversion factor for UNIX vs. Windows base time */
  return x.QuadPart;
}

static inline LONGLONG
systime ()
{
  return systime_ns () / 10;
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

  GetProcessTimes (hMainProc, &creation_time, &exit_time,
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

  tz = tz;			/* silence warning about unused variable */

  ptm = gmtime (&tv->tv_sec);
  st.wYear	   = ptm->tm_year + 1900;
  st.wMonth	   = ptm->tm_mon + 1;
  st.wDayOfWeek    = ptm->tm_wday;
  st.wDay	   = ptm->tm_mday;
  st.wHour	   = ptm->tm_hour;
  st.wMinute       = ptm->tm_min;
  st.wSecond       = ptm->tm_sec;
  st.wMilliseconds = tv->tv_usec / 1000;

  res = !SetSystemTime (&st);

  syscall_printf ("%d = settimeofday (%x, %x)", res, tv, tz);

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

hires_ms NO_COPY gtod;

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

  syscall_printf ("%d = time (%x)", res, ptr);

  return res;
}

/*
 * localtime_r.c
 * Original Author:	Adapted from tzcode maintained by Arthur David Olson.
 *
 * Converts the calendar time pointed to by tim_p into a broken-down time
 * expressed as local time. Returns a pointer to a structure containing the
 * broken-down time.
 */

#define SECSPERMIN	60
#define MINSPERHOUR	60
#define HOURSPERDAY	24
#define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY	(SECSPERHOUR * HOURSPERDAY)
#define DAYSPERWEEK	7
#define MONSPERYEAR	12

#define YEAR_BASE	1900
#define EPOCH_YEAR      1970
#define EPOCH_WDAY      4

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

#if 0 /* POSIX_LOCALTIME */

static _CONST int mon_lengths[2][MONSPERYEAR] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static _CONST int year_lengths[2] = {
  365,
  366
};

/*
 * Convert a time_t into a struct tm *.
 * Does NO timezone conversion.
 */

/* Cygwin internal */
static struct tm * __stdcall
corelocaltime (const time_t * tim_p)
{
  long days, rem;
  int y;
  int yleap;
  _CONST int *ip;
  struct tm &localtime_buf=_my_tls.locals.localtime_buf;

  time_t tim = *tim_p;
  struct tm *res = &localtime_buf;

  days = ((long) tim) / SECSPERDAY;
  rem = ((long) tim) % SECSPERDAY;

  while (rem < 0)
    {
      rem += SECSPERDAY;
      --days;
    }
  while (rem >= SECSPERDAY)
    {
      rem -= SECSPERDAY;
      ++days;
    }

  /* compute hour, min, and sec */
  res->tm_hour = (int) (rem / SECSPERHOUR);
  rem %= SECSPERHOUR;
  res->tm_min = (int) (rem / SECSPERMIN);
  res->tm_sec = (int) (rem % SECSPERMIN);

  /* compute day of week */
  if ((res->tm_wday = ((EPOCH_WDAY + days) % DAYSPERWEEK)) < 0)
    res->tm_wday += DAYSPERWEEK;

  /* compute year & day of year */
  y = EPOCH_YEAR;
  if (days >= 0)
    {
      for (;;)
	{
	  yleap = isleap (y);
	  if (days < year_lengths[yleap])
	    break;
	  y++;
	  days -= year_lengths[yleap];
	}
    }
  else
    {
      do
	{
	  --y;
	  yleap = isleap (y);
	  days += year_lengths[yleap];
	} while (days < 0);
    }

  res->tm_year = y - YEAR_BASE;
  res->tm_yday = days;
  ip = mon_lengths[yleap];
  for (res->tm_mon = 0; days >= ip[res->tm_mon]; ++res->tm_mon)
    days -= ip[res->tm_mon];
  res->tm_mday = days + 1;

  /* set daylight saving time flag */
  res->tm_isdst = -1;

  syscall_printf ("%d = corelocaltime (%x)", res, tim_p);

  return (res);
}

/* localtime: POSIX 8.1.1, C 4.12.3.4 */
/*
 * localtime takes a time_t (which is in UTC)
 * and formats it into a struct tm as a local time.
 */
extern "C" struct tm *
localtime (const time_t *tim_p)
{
  time_t tim = *tim_p;
  struct tm *rtm;

  tzset ();

  tim -= _timezone;

  rtm = corelocaltime (&tim);

  rtm->tm_isdst = _daylight;

  syscall_printf ("%x = localtime (%x)", rtm, tim_p);

  return rtm;
}

/* gmtime: C 4.12.3.3 */
/*
 * gmtime takes a time_t (which is already in UTC)
 * and just puts it into a struct tm.
 */
extern "C" struct tm *
gmtime (const time_t *tim_p)
{
  time_t tim = *tim_p;

  struct tm *rtm = corelocaltime (&tim);
  /* UTC has no daylight savings time */
  rtm->tm_isdst = 0;

  syscall_printf ("%x = gmtime (%x)", rtm, tim_p);

  return rtm;
}

#endif /* POSIX_LOCALTIME */

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
	      debug_printf ("got %d error from build_fh_name", fh->error ());
	      set_errno (fh->error ());
	    }
	}

      res = fh->utimens (tvp);

      if (!fromfd)
	delete fh;
    }

error:
  syscall_printf ("%d = utimes (%S, %p)",
		  res, win32.get_nt_native_path (), tvp);
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
  syscall_printf ("%d = futimens (%d, %p)", res, fd, tvp);
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

/* obsolete, changed to cygwin_tzset when localtime.c was added - dj */
extern "C" void
cygwin_tzset ()
{
}

#define stupid_printf if (cygwin_finished_initializing) debug_printf
void
hires_us::prime ()
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

  primed_ft.QuadPart = systime ();
  freq = (double) ((double) 1000000. / (double) ifreq.QuadPart);
  inited = true;
  SetThreadPriority (GetCurrentThread (), priority);
}

LONGLONG
hires_us::usecs (bool justdelta)
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
  LONGLONG res = justdelta ? now.QuadPart : primed_ft.QuadPart + now.QuadPart;
  return res;
}

void
hires_ms::prime ()
{
  if (!inited)
    {
      int priority = GetThreadPriority (GetCurrentThread ());
      SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);
      initime_ns = systime_ns () - (((LONGLONG) timeGetTime ()) * 10000LL);
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
  LONGLONG res = initime_ns + (((LONGLONG) timeGetTime ()) * 10000LL);
  if (res < (t - 40 * 10000LL))
    {
      inited = false;
      prime ();
      res = initime_ns + (((LONGLONG) timeGetTime ()) * 10000LL);
    }
  return res;
}

extern "C" int
clock_gettime (clockid_t clk_id, struct timespec *tp)
{
  if (clk_id != CLOCK_REALTIME)
    {
      set_errno (ENOSYS);
      return -1;
    }

  LONGLONG now = gtod.nsecs ();
  if (now == (LONGLONG) -1)
    return -1;

  tp->tv_sec = now / NSPERSEC;
  tp->tv_nsec = (now % NSPERSEC) * (1000000000 / NSPERSEC);
  return 0;
}

static DWORD minperiod;	// FIXME: Maintain period after a fork.

UINT
hires_ms::resolution ()
{
  if (!minperiod)
    {
      /* Try to empirically determine current timer resolution */
      int priority = GetThreadPriority (GetCurrentThread ());
      SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);
      DWORD period = 0;
      for (int i = 0; i < 4; i++)
	{
	  DWORD now;
	  DWORD then = timeGetTime ();
	  while ((now = timeGetTime ()) == then)
	    continue;
	  then = now;
	  while ((now = timeGetTime ()) == then)
	    continue;
	  period += now - then;
	}
      SetThreadPriority (GetCurrentThread (), priority);
      period /= 4;
      minperiod = period;
    }
  return minperiod;
}

extern "C" int
clock_getres (clockid_t clk_id, struct timespec *tp)
{
  if (clk_id != CLOCK_REALTIME)
    {
      set_errno (ENOSYS);
      return -1;
    }

  DWORD period = gtod.resolution ();

  tp->tv_sec = period / 1000;
  tp->tv_nsec = (period % 1000) * 1000000;

  return 0;
}

extern "C" int
clock_setres (clockid_t clk_id, struct timespec *tp)
{
  static NO_COPY bool period_set;
  if (clk_id != CLOCK_REALTIME)
    {
      set_errno (ENOSYS);
      return -1;
    }

  if (period_set)
    timeEndPeriod (minperiod);

  DWORD period = (tp->tv_sec * 1000) + ((tp->tv_nsec) / 1000000);

  if (timeBeginPeriod (period))
    {
      minperiod = period;
      period_set = true;
    }
  else
    {
      __seterrno ();
      timeBeginPeriod (minperiod);
      return -1;
    }

  return 0;
}
