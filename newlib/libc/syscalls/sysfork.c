/* connector for fork */

/* Don't define this if NO_FORK.  See for example libc/sys/win32/spawn.c.  */

#ifndef NO_FORK

#include <reent.h>

int
fork ()
{
#ifdef REENTRANT_SYSCALLS_PROVIDED
  return _fork_r (_REENT);
#else
  return _fork ();
#endif
}

#endif
