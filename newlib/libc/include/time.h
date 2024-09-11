/*
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.
c) UNIX System Laboratories, Inc.
All or some portions of this file are derived from material licensed
to the University of California by American Telephone and Telegraph
Co. or Unix System Laboratories, Inc. and are reproduced herein with
the permission of UNIX System Laboratories, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
/*
 * time.h
 * 
 * Struct and function declarations for dealing with time.
 */

#ifndef _TIME_H_
#define _TIME_H_

#include <sys/cdefs.h>

#define __need_size_t
#define __need_NULL
#include <stddef.h>
#include <sys/_types.h>
#include <sys/_timespec.h>

_BEGIN_STD_C

#ifndef _CLOCKS_PER_SEC_
#define _CLOCKS_PER_SEC_ 1000000
#endif

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC _CLOCKS_PER_SEC_
#endif

#ifndef TIME_UTC
#define TIME_UTC 1
#endif

#ifndef _CLOCK_T_DECLARED
typedef	_CLOCK_T_	clock_t;
#define	_CLOCK_T_DECLARED
#endif

struct tm
{
  int	tm_sec;
  int	tm_min;
  int	tm_hour;
  int	tm_mday;
  int	tm_mon;
  int	tm_year;
  int	tm_wday;
  int	tm_yday;
  int	tm_isdst;
#ifdef __TM_GMTOFF
  long	__TM_GMTOFF;
#endif
#ifdef __TM_ZONE
  const char *__TM_ZONE;
#endif
};

#if __POSIX_VISIBLE

#ifndef CLK_TCK
#define CLK_TCK CLOCKS_PER_SEC
#endif

#define CLOCK_REALTIME		((clockid_t) 1)

/* Flag indicating time is "absolute" with respect to the clock
   associated with a time.  Value 4 is historic. */

#define TIMER_ABSTIME	4

#include <sys/_locale.h>

#ifndef _CLOCKID_T_DECLARED
typedef	__clockid_t	clockid_t;
#define	_CLOCKID_T_DECLARED
#endif

#ifndef _TIMER_T_DECLARED
typedef	__timer_t	timer_t;
#define	_TIMER_T_DECLARED
#endif

/*
 * Structure defined by POSIX.1b to be like a itimerval, but with
 * timespecs. Used in the timer_*() system calls.
 */
struct itimerspec {
	struct timespec  it_interval;
	struct timespec  it_value;
};

#endif

#if __GNU_VISIBLE
#define CLOCK_REALTIME_COARSE	((clockid_t) 0)
#endif

#if defined(_POSIX_CPUTIME)

/* When used in a clock or timer function call, this is interpreted as
   the identifier of the CPU_time clock associated with the PROCESS
   making the function call.  */

#define CLOCK_PROCESS_CPUTIME_ID ((clockid_t) 2)

#endif

#if defined(_POSIX_THREAD_CPUTIME)

/*  When used in a clock or timer function call, this is interpreted as
    the identifier of the CPU_time clock associated with the THREAD
    making the function call.  */

#define CLOCK_THREAD_CPUTIME_ID	((clockid_t) 3)

#endif

#if defined(_POSIX_MONOTONIC_CLOCK)

/*  The identifier for the system-wide monotonic clock, which is defined
 *  as a clock whose value cannot be set via clock_settime() and which
 *  cannot have backward clock jumps. */

#define CLOCK_MONOTONIC		((clockid_t) 4)

#endif

#if __GNU_VISIBLE

#define CLOCK_MONOTONIC_RAW	((clockid_t) 5)

#define CLOCK_MONOTONIC_COARSE	((clockid_t) 6)

#define CLOCK_BOOTTIME		((clockid_t) 7)

#define CLOCK_REALTIME_ALARM	((clockid_t) 8)

#define CLOCK_BOOTTIME_ALARM	((clockid_t) 9)

#endif

/* defines for the opengroup specifications Derived from Issue 1 of the SVID.  */
#if __SVID_VISIBLE || __XSI_VISIBLE
extern long _timezone;
extern int _daylight;
#endif

#if __POSIX_VISIBLE
extern char *tzname[2];
#endif /* __POSIX_VISIBLE */

char	  *asctime (const struct tm *_tblock);

#if __POSIX_VISIBLE
#define __ASCTIME_SIZE 26

char	  *asctime_r 	(const struct tm *__restrict,
                         char [__restrict_arr __min_size(__ASCTIME_SIZE)]);
#endif

clock_t	   clock (void);

#if defined(_POSIX_CPUTIME)
int        clock_getcpuclockid (pid_t pid, clockid_t *clock_id);
#endif /* _POSIX_CPUTIME */

#if __POSIX_VISIBLE
int        clock_getres (clockid_t clock_id, struct timespec *res);

int        clock_gettime (clockid_t clock_id, struct timespec *tp);

int        clock_nanosleep (clockid_t clock_id, int flags,
                            const struct timespec *rqtp,
                            struct timespec *rmtp);

int        clock_settime (clockid_t clock_id, const struct timespec *tp);
#endif

char	  *ctime (const time_t *_time);

#if __POSIX_VISIBLE
char	  *ctime_r 	(const time_t *,
                         char [__restrict_arr __min_size(__ASCTIME_SIZE)]);
#endif

double	   difftime (time_t _time2, time_t _time1);

#if __XSI_VISIBLE >= 4
struct tm *getdate (const char *);
#endif /* __XSI_VISIBLE >= 4 */

#if __GNU_VISIBLE
extern int getdate_err;

int	   getdate_r (const char *, struct tm *);
#endif /* __GNU_VISIBLE */

struct tm *gmtime (const time_t *_timer);

#if __POSIX_VISIBLE
struct tm *gmtime_r (const time_t *__restrict,
                     struct tm *__restrict);
#endif

struct tm *localtime (const time_t *_timer);

#if __POSIX_VISIBLE
struct tm *localtime_r 	(const time_t *__restrict,
				 struct tm *__restrict);
#endif

time_t	   mktime (struct tm *_timeptr);

int        nanosleep (const struct timespec  *rqtp, struct timespec *rmtp);

size_t	   strftime (char *__restrict _s,
                     size_t _maxsize, const char *__restrict _fmt,
                     const struct tm *__restrict _t);

int        timespec_get(struct timespec *_ts, int _base);

#if __POSIX_VISIBLE
size_t strftime_l (char *__restrict _s, size_t _maxsize,
			  const char *__restrict _fmt,
			  const struct tm *__restrict _t, locale_t _l);
#endif

#if __XSI_VISIBLE
char      *strptime (const char *__restrict,
				 const char *__restrict,
				 struct tm *__restrict);
#endif
#if __GNU_VISIBLE
char      *strptime_l (const char *__restrict, const char *__restrict,
                       struct tm *__restrict, locale_t);
#endif

time_t	   time (time_t *_timer);

#if __BSD_VISIBLE || __SVID_VISIBLE || __GNU_VISIBLE
time_t	   timegm (struct tm *_timeptr);
#endif

#if __POSIX_VISIBLE
struct sigevent;
int        timer_create (clockid_t clock_id,
                         struct sigevent *__restrict evp,
                         timer_t *__restrict timerid);

int        timer_delete (timer_t timerid);

int        timer_getoverrun (timer_t timerid);

int        timer_gettime (timer_t timerid, struct itimerspec *value);

int        timer_settime (timer_t timerid, int flags,
                          const struct itimerspec *__restrict value,
                          struct itimerspec *__restrict ovalue);
#endif

void       tzset (void);

#if __STDC_WANT_LIB_EXT1__ == 1
#include <sys/_types.h>

#ifndef _ERRNO_T_DEFINED
typedef __errno_t errno_t;
#define _ERRNO_T_DEFINED
#endif

#ifndef _RSIZE_T_DEFINED
typedef __rsize_t rsize_t;
#define _RSIZE_T_DEFINED
#endif
#endif

_END_STD_C

#endif /* _TIME_H_ */
