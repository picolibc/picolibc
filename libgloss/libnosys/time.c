/*
 * Stub version of times.
 */

#include <time.h>
#include <errno.h>
#undef errno
extern int errno;
#include "warning.h"

int __attribute__((weak)) clock_gettime(clockid_t clock_id, struct timespec *t_time)
{
  errno = ENOSYS;
  return -1;
}

stub_warning (clock_gettime)
