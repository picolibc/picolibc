/*
 * Stub version of getentropy.
 */

#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#undef errno
extern int errno;
#include "warning.h"

int
_getentropy (void *buf,
        size_t buflen)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_getentropy)
