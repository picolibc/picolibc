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

int
_write (int   file,
        char *ptr,
        int   len)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_write)

