/*
 * Stub version of gettimeofday.
 */

#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#undef errno
extern int errno;
#include "warning.h"

struct timeval;

#ifndef REENTRANT_SYSCALLS_STUBS

int
_gettimeofday (struct timeval  *ptimeval,
        void *ptimezone)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_gettimeofday)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_gettimeofday_r (struct _reent *ptr,
     struct timeval *ptimeval,
     void *ptimezone)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_gettimeofday_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
