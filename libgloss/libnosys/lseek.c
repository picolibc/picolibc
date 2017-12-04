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

int
_DEFUN (_lseek, (file, ptr, dir),
        int   file,
        int   ptr,
        int   dir)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_lseek)
