/* time.h -- An implementation of the standard Unix <sys/time.h> file.
   Written by Geoffrey Noer <noer@cygnus.com>
   Public domain; no rights reserved. */

#ifndef _SYS_TIME_H_
#define _SYS_TIME_H_

#include <_ansi.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WINSOCK_H
struct timeval {
  long tv_sec;
  long tv_usec;
};

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

#ifdef __CYGWIN__
#include <sys/select.h>
#endif /* __CYGWIN__ */

#endif /* _WINSOCK_H */

#define ITIMER_REAL     0
#define ITIMER_VIRTUAL  1
#define ITIMER_PROF     2

struct  itimerval {
  struct  timeval it_interval;
  struct  timeval it_value;
};

int _EXFUN(gettimeofday, (struct timeval *__p, struct timezone *__z));
int _EXFUN(settimeofday, (const struct timeval *, const struct timezone *));
int _EXFUN(utimes, (const char *__path, struct timeval *__tvp));
int _EXFUN(getitimer, (int __which, struct itimerval *__value));
int _EXFUN(setitimer, (int __which, const struct itimerval *__value,
					struct itimerval *__ovalue));

#ifdef __cplusplus
}
#endif
#endif /* _SYS_TIME_H_ */
