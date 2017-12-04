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

int
_kill (int pid,
        int sig)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_kill)
