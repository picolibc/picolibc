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

int
_open (char *file,
        int   flags,
        int   mode)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_open)
