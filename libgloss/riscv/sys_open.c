#include <machine/syscall.h>
#include "internal_syscall.h"

/* Open a file.  */
int
_open(const char *name, int flags, int mode)
{
  return syscall_errno (SYS_open, 3, name, flags, mode, 0, 0, 0);
}
