/*
 * Stub version of getpid.
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
_getpid (void)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_getpid)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_getpid_r (struct _reent *ptr)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_getpid_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
