/*
 * Stub version of open.
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
_open (char *file,
        int   flags,
        int   mode)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_open)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_open_r (struct _reent *ptr,
     const char *file,
     int flags,
     int mode)
{
  errno = ENOSYS;
  return -1;
}

stub_warning (_open_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
