/* times.cc

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <time.h>
#include <sys/times.h>
#include <sys/timeb.h>
#include <utime.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "cygerrno.h"
#include "fhandler.h"
#include "path.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "security.h"

#define FACTOR (0x19db1ded53e8000LL)
#define NSPERSEC 10000000LL

static void __stdcall timeval_to_filetime (timeval *time, FILETIME *out);

/* Cygwin internal */
static unsigned long long __stdcall
__to_clock_t (FILETIME * src, int flag)
{
  unsigned long long total = ((unsigned long long) src->dwHighDateTime << 32) + ((unsigned)src->dwLowDateTime);
  syscall_printf ("dwHighDateTime %u, dwLowDateTime %u", src->dwHighDateTime, src->dwLowDateTime);

  /* Convert into clock ticks - the total is in 10ths of a usec.  */
  if (flag)
    total -= FACTOR;

  total /= (unsigned long long) (NSPERSEC / CLOCKS_PER_SEC);
  syscall_printf ("total %08x %08x\n", (unsigned)(total>>32), (unsigned)(total));
  return total;
}

/* times: POSIX 4.5.2.1 */
extern "C" clock_t
times (struct tms * buf)
{
  FILETIME creation_time, exit_time, kernel_time, user_time;

  DWORD ticks = GetTickCount ();
  /* Ticks is in milliseconds, convert to our ticks. Use long long to prevent
     overflow. */
  clock_t tc = (clock_t) ((long long) ticks * CLOCKS_PER_SEC / 1000);
  if (os_being_run == winNT)
    {
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
    }
  else
    /* GetProcessTimes() does not work for non-NT versions of Windows.  The
       return values are undefined, so instead just copy the ticks value
       into utime so that clock() will work properly on these systems */
    {
      buf->tms_utime = tc;
      buf->tms_stime = 0;
      buf->tms_cstime = 0;
      buf->tms_cutime = 0;
    }

   return tc;
}

extern "C" clock_t
_times (struct tms * buf)
{
  return times (buf);
}

/* settimeofday: BSD */
extern "C" int
settimeofday (const struct timeval *, const struct timezone *)
{
  set_errno (ENOSYS);
  return -1;
}

/* timezone: standards? */
extern "C" char *
timezone ()
{
#ifdef _MT_SAFE
  char *b=_reent_winsup()->_b;
#else
  static NO_COPY char b[20] = {0};
#endif

  tzset();
  __small_sprintf (b,"GMT%+d:%02d", (int) (-_timezone / 3600), (int) (abs(_timezone / 60) % 60));
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

/* gettimeofday: BSD */
extern "C" int
gettimeofday (struct timeval *p, struct timezone *z)
{
  int res = 0;

  if (p != NULL)
    {
      FILETIME f;

      GetSystemTimeAsFileTime (&f);
      totimeval (p, &f, 0, 1);
    }

  if (z != NULL)
    {
      tzset();
      z->tz_minuteswest = _timezone / 60;
      z->tz_dsttime = _daylight;
    }

  syscall_printf ("%d = gettimeofday (%x, %x)", res, p, z);

  return res;
}

extern "C"
int
_gettimeofday (struct timeval *p, struct timezone *z)
{
  return gettimeofday (p, z);
}

#if 0
/* Work out magic constant below */
genf ()
{
  SYSTEMTIME s;
  FILETIME f;
  s.wYear = 1970;
  s.wMonth = 1;
  s.wDayOfWeek = 4;
  s.wDay = 1;
  s.wHour = 0;
  s.wMinute = 0;
  s.wSecond = 0;
  s.wMilliseconds = 0;
  SystemTimeToFileTime (&s, &f);

  small_printf ("FILE TIME is %08x%08x\n",
	       f.dwHighDateTime,
	       f.dwLowDateTime);
}
#endif

/* Cygwin internal */
void
time_t_to_filetime (time_t time_in, FILETIME *out)
{
  long long x = time_in * NSPERSEC + FACTOR;
  out->dwHighDateTime = x >> 32;
  out->dwLowDateTime = x;
}

/* Cygwin internal */
static void __stdcall
timeval_to_filetime (timeval *time_in, FILETIME *out)
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
/* Convert a Win32 time to "UNIX" format. */
long __stdcall
to_time_t (FILETIME *ptr)
{
  /* A file time is the number of 100ns since jan 1 1601
     stuffed into two long words.
     A time_t is the number of seconds since jan 1 1970.  */

  long rem;
  long long x = ((long long) ptr->dwHighDateTime << 32) + ((unsigned)ptr->dwLowDateTime);
  x -= FACTOR;			/* number of 100ns between 1601 and 1970 */
  rem = x % ((long long)NSPERSEC);
  rem += (NSPERSEC / 2);
  x /= (long long) NSPERSEC;		/* number of 100ns in a second */
  x += (long long) (rem / NSPERSEC);
  return x;
}

/* time: POSIX 4.5.1.1, C 4.12.2.4 */
/* Return number of seconds since 00:00 UTC on jan 1, 1970 */
extern "C"
time_t
time (time_t * ptr)
{
  time_t res;
  SYSTEMTIME systemtime;
  FILETIME filetime;

  GetSystemTime (&systemtime);
  SystemTimeToFileTime (&systemtime, &filetime);
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
#ifdef _MT_SAFE
  struct tm &localtime_buf=_reent_winsup()->_localtime_buf;
#else
  static NO_COPY struct tm localtime_buf = {0};
#endif

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
extern "C"
struct tm *
localtime (const time_t *tim_p)
{
  time_t tim = *tim_p;
  struct tm *rtm;

  tzset();

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
extern "C"
struct tm *
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

/* utimes: standards? */
extern "C"
int
utimes (const char *path, struct timeval *tvp)
{
  int res = 0;
  struct timeval tmp[2];
  path_conv win32 (path);

  if (win32.error)
    {
      set_errno (win32.error);
      syscall_printf ("-1 = utimes (%s, %x)", path, tvp);
      return -1;
    }

  /* MSDN suggests using FILE_FLAG_BACKUP_SEMANTICS for accessing
     the times of directories.  FIXME: what about Win95??? */
  HANDLE h = CreateFileA (win32.get_win32 (),
			  GENERIC_WRITE,
			  FILE_SHARE_READ | FILE_SHARE_WRITE,
			  &sec_none_nih,
			  OPEN_EXISTING,
			  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
			  0);

  if (h == INVALID_HANDLE_VALUE)
    {
      if ((res = GetFileAttributes (win32.get_win32 ())) != -1 &&
		(res & FILE_ATTRIBUTE_DIRECTORY))
	{
	  /* What we can do with directories more? */
	  res = 0;
	}
      else
	{
	  res = -1;
	  __seterrno ();
	}
    }
  else
    {
      if (tvp == 0)
	{
	  gettimeofday (&tmp[0], 0);
	  tmp[1] = tmp[0];
	  tvp = tmp;
	}

      FILETIME lastaccess;
      FILETIME lastwrite;

      timeval_to_filetime (tvp + 0, &lastaccess);
      timeval_to_filetime (tvp + 1, &lastwrite);

      debug_printf ("incoming lastaccess %08x %08x",
		   tvp->tv_sec,
		   tvp->tv_usec);

//      dump_filetime (lastaccess);
//      dump_filetime (lastwrite);

      /* FIXME: SetFileTime needs a handle with a write lock
	 on the file whose time is being modified.  So calls to utime()
	 fail for read only files.  */

      if (!SetFileTime (h, 0, &lastaccess, &lastwrite))
	{
	  __seterrno ();
	  res = -1;
	}
      else
	res = 0;
      CloseHandle (h);
    }

  syscall_printf ("%d = utimes (%s, %x); (h%d)",
		  res, path, tvp, h);
  return res;
}

/* utime: POSIX 5.6.6.1 */
extern "C"
int
utime (const char *path, struct utimbuf *buf)
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
extern "C"
int
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
extern "C"
void
cygwin_tzset ()
{
}
