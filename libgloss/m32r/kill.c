#include <sys/types.h>
#include <sys/stat.h>
#include "syscall.h"
#include "eit.h"

int
_kill (int n, int m)
{
  return TRAP0 (SYS_exit, 0xdead, 0, 0);
}
