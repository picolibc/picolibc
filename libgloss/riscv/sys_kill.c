#include <machine/syscall.h>
#include "internal_syscall.h"

/* Send a signal. Minimal implementation for a system without processes
   just causes an error.  */
int
_kill(int pid, int sig)
{
  errno = EINVAL;
  return -1;
}
