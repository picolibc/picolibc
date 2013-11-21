#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int stat(const char *__restrict file, struct stat *__restrict st)
{
  errno = ENOSYS;
  return -1;
}
