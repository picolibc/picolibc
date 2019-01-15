/* sys/timerfd.h: define timerfd_create(2) and friends

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef	_SYS_TIMERFD_H
#define	_SYS_TIMERFD_H

#include <time.h>
#include <sys/_default_fcntl.h>
#include <asm/socket.h>

enum
{
  /* timerfd_create */
  TFD_CLOEXEC = O_CLOEXEC,
  TFD_NONBLOCK = O_NONBLOCK,
  /* timerfd_settime */
  TFD_TIMER_ABSTIME = TIMER_ABSTIME,
  TFD_TIMER_CANCEL_ON_SET = (TIMER_ABSTIME << 1)
};
#define TFD_CLOEXEC TFD_CLOEXEC
#define TFD_NONBLOCK TFD_NONBLOCK
#define TFD_TIMER_ABSTIME TFD_TIMER_ABSTIME
#define TFD_TIMER_CANCEL_ON_SET TFD_TIMER_CANCEL_ON_SET

#define TFD_IOC_SET_TICKS	_IOW('T', 0, __uint64_t)

#ifdef __cplusplus
extern "C" {
#endif

extern int timerfd_create (clockid_t, int);
extern int timerfd_settime (int, int, const struct itimerspec *,
			    struct itimerspec *);
extern int timerfd_gettime (int, struct itimerspec *);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_TIMERFD_H */
