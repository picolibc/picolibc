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

int
_execve (char  *name,
        char **argv,
        char **env)
{
  errno = ENOSYS;
  return -1;
}

stub_warning(_execve)
