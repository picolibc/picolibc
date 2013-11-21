#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "trap.h"


int
stat (const char *__restrict path, struct stat *__restrict st)

{
  return TRAP0 (SYS_stat, path, st, 0);
}
