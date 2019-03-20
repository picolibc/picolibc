/*
 * Stub version of link.
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
_link (char *existing,
        char *new)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_link)

#else /* REENTRANT_SYSCALLS_STUBS */

int
_link_r (struct _reent *ptr,
     const char *old,
     const char *new)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_link_r)

#endif /* REENTRANT_SYSCALLS_STUBS */
