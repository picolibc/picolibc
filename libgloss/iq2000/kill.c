#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "trap.h"


int
_kill (int n, int m)
{
  return TRAP0 (SYS_exit, 0xdead, 0, 0);
}

