/*
 * Stub version of pthread_setcancelstate.
 */

#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#undef errno
extern int errno;
#include "warning.h"

int
pthread_setcancelstate (int state, int *oldstate)
{
  return -1;
}

stub_warning(pthread_setcancelstate)
