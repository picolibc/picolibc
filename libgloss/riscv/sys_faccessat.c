#include <machine/syscall.h>
#include "internal_syscall.h"

/* Permissions of a file (by name) in a given directory.  */
int _faccessat(int dirfd, const char *file, int mode, int flags)
{
  return syscall_errno (SYS_faccessat, dirfd, file, mode, flags, 0, 0);
}
