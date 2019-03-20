/*
 * Stub version of lseek.
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
_lseek (int   file,
        int   ptr,
        int   dir)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_lseek)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_lseek_r (struct _reent *ptr,
     int fd,
     _off_t pos,
     int whence)
{
  errno = ENOSYS;
  return -1;
}

stub_warning (_lseek_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
