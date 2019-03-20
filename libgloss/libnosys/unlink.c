/*
 * Stub version of unlink.
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
_unlink (char *name)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_unlink)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_unlink_r (struct _reent *ptr,
     const char *file)
{
  errno = ENOSYS;
  return -1;
}

stub_warning (_unlink_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
