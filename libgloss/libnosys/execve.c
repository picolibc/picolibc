/*
 * Stub version of execve.
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
_execve (char  *name,
        char **argv,
        char **env)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_execve)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_execve_r (struct _reent *ptr,
     const char *name,
     char *const argv[],
     char *const env[])
{
  errno = ENOSYS;
  return -1;
}

stub_warning (_execve_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
