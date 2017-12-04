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

int
_link (char *existing,
        char *new)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_link)
