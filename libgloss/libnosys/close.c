/*
 * Stub version of close.
 */

#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#undef errno
extern int errno;
#include "warning.h"

#ifndef REENTRANT_SYSCALLS_STUBS

int
_close (int fildes)
{
  errno = ENOSYS;
  return -1;
}

stub_warning (_close)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_close_r(struct _reent *ptr, int fd)
{
  errno = ENOSYS;
  return -1;
}

stub_warning (_close_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
