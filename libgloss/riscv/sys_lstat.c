#include <machine/syscall.h>
#include <sys/stat.h>
#include "internal_syscall.h"
#include "kernel_stat.h"

/* Status of a link (by name).  */
int _lstat(const char *file, struct stat *st)
{
  struct kernel_stat kst;
  int rv = syscall_errno (SYS_lstat, file, &kst, 0, 0, 0, 0);
  _conv_stat (st, &kst);
  return rv;
}
