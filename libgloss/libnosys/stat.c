/*
 * Stub version of stat.
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
_stat (const char  *file,
        struct stat *st)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_stat)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_stat_r (struct _reent *ptr,
     const char *file,
     struct stat *pstat)
{
  errno = ENOSYS;
  return -1;
}

stub_warning (_stat_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
