/*
 * Stub version of kill.
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
_kill (int pid,
        int sig)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_kill)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_kill_r (struct _reent *ptr,
        int pid,
        int sig)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_kill_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
