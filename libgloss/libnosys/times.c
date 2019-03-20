/*
 * Stub version of times.
 */

#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <sys/times.h>
#include <errno.h>
#undef errno
extern int errno;
#include "warning.h"

#ifndef REENTRANT_SYSCALLS_STUBS

clock_t
_times (struct tms *buf)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_times)

#else /* REENTRANT_SYSCALLS_STUBS */

clock_t
_times_r (struct _reent *ptr,
     struct tms *ptms)
{
  errno = ENOSYS;
  return -1;
}

stub_warning (_times_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
