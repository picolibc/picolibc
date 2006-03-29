/* libc/sys/linux/time.c - Time-related system calls */

/* Written 2000 by Werner Almesberger */


#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/times.h>
#include <sys/syscall.h>


_syscall1(time_t,time,time_t *,t)
_syscall1(clock_t,times,struct tms *,buf)
_syscall2(int,gettimeofday,struct timeval *,tv,struct timezone *,tz)
_syscall2(int,nanosleep,const struct timespec *,req,struct timespec *,rem)


unsigned int sleep(unsigned int seconds)
{
    struct timespec ts;

    ts.tv_sec = seconds;
    ts.tv_nsec = 0;
    if (!nanosleep(&ts,&ts)) return 0;
    if (errno == EINTR) return ts.tv_sec;
    return -1;
}
