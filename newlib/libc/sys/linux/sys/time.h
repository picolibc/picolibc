/* libc/sys/linux/sys/time.h - Time handling */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <sys/types.h>
#include <linux/time.h>

/* --- redundant stuff below --- */

#include <_ansi.h>

int _EXFUN(gettimeofday, (struct timeval *__p, struct timezone *__z));
int _EXFUN(settimeofday, (const struct timeval *, const struct timezone *));
int _EXFUN(utimes, (const char *__path, struct timeval *__tvp));
int _EXFUN(getitimer, (int __which, struct itimerval *__value));
int _EXFUN(setitimer, (int __which, const struct itimerval *__value,
                                        struct itimerval *__ovalue));
#endif
