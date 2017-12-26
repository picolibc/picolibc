#include <machine/syscall.h>
#include "internal_syscall.h"

/* Close a file.  */
int
_close(int file)
{
  return syscall_errno (SYS_close, file, 0, 0, 0, 0, 0);
}
