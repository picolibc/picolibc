#include <machine/syscall.h>
#include "kernel_stat.h"
#include "internal_syscall.h"

/* Status of an open file. The sys/stat.h header file required is
   distributed in the include subdirectory for this C library.  */
int
_fstatat(int dirfd, const char *file, struct stat *st, int flags)
{
  struct kernel_stat kst;
  int rv = syscall_errno (SYS_fstatat, dirfd, file, &kst, flags, 0, 0);
  _conv_stat (st, &kst);
  return rv;
}
