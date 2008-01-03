#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "trap.h"


_isatty (fd)
     int fd;
{
  return 1;
}
