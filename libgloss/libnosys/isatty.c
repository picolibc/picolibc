/*
 * Stub version of isatty.
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
_isatty (int file)
{
  errno = ENOSYS;
  return 0;
}

stub_warning(_isatty)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_isatty_r (ptr, fd)
     struct _reent *ptr;
     int fd;
{
  errno = ENOSYS;
  return 0;
}

stub_warning(_isatty_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
