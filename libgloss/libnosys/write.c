/*
 * Stub version of write.
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
_write (int   file,
        char *ptr,
        int   len)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_write)

#else /* REENTRANT_SYSCALLS_STUBS */

_ssize_t
_write_r (struct _reent *ptr,
     int fd,
     const void *buf,
     size_t cnt)
{
  errno = ENOSYS;
  return -1;
}

stub_warning (_write_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
