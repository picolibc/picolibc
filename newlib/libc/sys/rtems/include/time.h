/*
 *  $Id$
 */


#ifndef __POSIX_TIME_h
#define __POSIX_TIME_h

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/features.h>
#include "_ansi.h"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/siginfo.h>

/*
 *  4.8.1.5 Special Symbol {CLK_TCK}, P1003.1b-1993, p. 97
 */

#define CLK_TCK  sysconf(_SC_CLK_TCK)

/*
 *  Get size_t from the GNU C version of stddef.h
 */

#define __need_size_t
#include <stddef.h>

/*
 *  ANSI C Time Conversion Structure
 *
 *  XXX reference
 */

struct tm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
};

/*
 *  ANSI C Time Conversion Routines
 *
 *  XXX reference
 */

clock_t    _EXFUN(clock,    (void));
double     _EXFUN(difftime, (time_t _time2, time_t _time1));
time_t     _EXFUN(mktime,   (struct tm *_timeptr));
time_t     _EXFUN(time,     (time_t *_timer));
size_t     _EXFUN(strftime,
            (char *_s, size_t _maxsize, const char *_fmt, const struct tm *_t));

#ifndef _REENT_ONLY
char      *_EXFUN(asctime,  (const struct tm *_tblock));
char      *_EXFUN(ctime,    (const time_t *_time));
struct tm *_EXFUN(gmtime,   (const time_t *_timer));
struct tm *_EXFUN(localtime,(const time_t *_timer));
#endif

/*
 *  Added by 8.3.5 - 8.3.8, P1002.1c/D10, p. 66-69.
 */

char      *_EXFUN(asctime_r,  (const struct tm *, char *));
char      *_EXFUN(ctime_r,  (const time_t *, char *));
struct tm *_EXFUN(gmtime_r, (const time_t *, struct tm *));
struct tm *_EXFUN(localtime_r,  (const time_t *, struct tm *));

/*
 *  4.5.1 Get System Time, P1003.1b-1993, p. 91
 */

time_t _EXFUN(time, (time_t *tloc));

#if defined(_POSIX_TIMERS)

/*
 *  14.2.1 Clocks, P1003.1b-1993, p. 263
 */

int _EXFUN(clock_settime, (clockid_t clock_id, const struct timespec *tp));
int _EXFUN(clock_gettime, (clockid_t clock_id, struct timespec *tp));
int _EXFUN(clock_getres,  (clockid_t clock_id, struct timespec *res));

/*
 *  14.2.2 Create a Per-Process Timer, P1003.1b-1993, p. 264
 */

int _EXFUN(timer_create,
  (clockid_t clock_id, struct sigevent *evp, timer_t *timerid)
);

/*
 *  14.2.3 Delete a Per_process Timer, P1003.1b-1993, p. 266
 */

int _EXFUN(timer_delete, (timer_t timerid));

/*
 *  14.2.4 Per-Process Timers, P1003.1b-1993, p. 267
 */

int _EXFUN(timer_settime,
  (timer_t timerid,
   int flags,
   const struct itimerspec *value,
   struct itimerspec       *ovalue)
);

int _EXFUN(timer_gettime, (timer_t timerid, struct itimerspec *value));
int _EXFUN(timer_getoverrun, (timer_t timerid));

/*
 *  14.2.5 High Resolution Sleep, P1003.1b-1993, p. 269
 */

int _EXFUN(nanosleep, (const struct timespec  *rqtp, struct timespec *rmtp));

#endif /* _POSIX_TIMERS */

/*
 *  20.1.1 CPU-time Clock Attributes, P1003.4b/D8, p. 54
 */

/* values for the clock enable attribute */

#define CLOCK_ENABLED  1  /* clock is enabled, i.e. counting execution time */
#define CLOCK_DISABLED 0  /* clock is disabled */

/* values for the pthread cputime_clock_allowed attribute */

#define CLOCK_ALLOWED    1 /* If a thread is created with this value a */
                           /*   CPU-time clock attached to that thread */
                           /*   shall be accessible. */
#define CLOCK_DISALLOWED 0 /* If a thread is created with this value, the */
                           /*   thread shall not have a CPU-time clock */
                           /*   accessible. */

/*
 *  14.1.3 Manifest Constants, P1003.1b-1993, p. 262
 */

#define CLOCK_REALTIME (clockid_t)1

/*
 *  Flag indicating time is "absolute" with respect to the clock
 *  associated with a time
 */

#define TIMER_ABSTIME  4

/*
 *  20.1.2 Manifest Constants, P1003.4b/D8, p. 55
 */

#if defined(_POSIX_CPUTIME)

/*
 *  When used in a clock or timer function call, this is interpreted as
 *  the identifier of the CPU_time clock associated with the PROCESS
 *  making the function call.
 */

#define CLOCK_PROCESS_CPUTIME (clockid_t)2

#endif

#if defined(_POSIX_THREAD_CPUTIME)

/*
 *  When used in a clock or timer function call, this is interpreted as
 *  the identifier of the CPU_time clock associated with the THREAD
 *  making the function call.
 */

#define CLOCK_THREAD_CPUTIME (clockid_t)3

#endif

#if defined(_POSIX_CPUTIME)

/*
 *  20.1.3 Accessing a Process CPU-time CLock, P1003.4b/D8, p. 55
 */

int _EXFUN(clock_getcpuclockid, (pid_t pid, clockid_t *clock_id));

#endif /* _POSIX_CPUTIME */

#if defined(_POSIX_CPUTIME) || defined(_POSIX_THREAD_CPUTIME)

/*
 *  20.1.5 CPU-time Clock Attribute Access, P1003.4b/D8, p. 56
 */

int _EXFUN(clock_setenable_attr, (clockid_t clock_id, int attr));

int _EXFUN(clock_getenable_attr, (clockid_t clock_id, int *attr));

#endif /* _POSIX_CPUTIME or _POSIX_THREAD_CPUTIME */

#ifdef __cplusplus
}
#endif

#endif
/* end of include file */
