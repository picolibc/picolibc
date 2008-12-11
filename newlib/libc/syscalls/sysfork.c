/* connector for fork */

/* Don't define this if NO_FORK.  See for example libc/sys/win32/spawn.c.  */

#ifndef NO_FORK

#include <reent.h>
#include <unistd.h>

int
_DEFUN_VOID (fork)
{
  return _fork_r (_REENT);
}

#endif
