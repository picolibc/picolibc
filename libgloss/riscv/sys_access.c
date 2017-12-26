#include <machine/syscall.h>
#include "internal_syscall.h"

/* Permissions of a file (by name).  */
int
_access(const char *file, int mode)
{
  return syscall_errno (SYS_access, file, mode, 0, 0, 0, 0);
}
