#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <sys/types.h>

int readlink(const char *__restrict path, char *__restrict buf, size_t bufsize)
{
  errno = ENOSYS;
  return -1;
}
