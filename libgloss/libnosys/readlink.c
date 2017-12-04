/*
 * Stub version of readlink.
 */

#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <sys/types.h>
#undef errno
extern int errno;
#include "warning.h"

int
_readlink (const char *path,
        char *buf,
        size_t bufsize)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_readlink)
