/*
 * Stub version of fstat.
 */

#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#undef errno
extern int errno;
#include "warning.h"

#ifndef REENTRANT_SYSCALLS_STUBS

int
_fstat (int          fildes,
        struct stat *st)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_fstat)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_fstat_r (ptr, fd, pstat)
     struct _reent *ptr;
     int fd;
     struct stat *pstat;
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_fstat_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
