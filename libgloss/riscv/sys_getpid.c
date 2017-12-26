#include <machine/syscall.h>

/* Get process id. This is sometimes used to generate strings unlikely
   to conflict with other processes. Minimal implementation for a
   system without processes just returns 1.  */

int
_getpid()
{
  return 1;
}
