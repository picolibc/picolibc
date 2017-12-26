#include <machine/syscall.h>
#include "kernel_stat.h"
#include "internal_syscall.h"

/* Status of an open file. The sys/stat.h header file required is
   distributed in the include subdirectory for this C library.  */

int
_fstat(int file, struct stat *st)
{
  struct kernel_stat kst;
  int rv = syscall_errno (SYS_fstat, file, &kst, 0, 0, 0, 0);
  _conv_stat (st, &kst);
  return rv;
}
