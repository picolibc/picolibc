#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "trap.h"


int
_isatty (int fd)
{
  return 1;
}
