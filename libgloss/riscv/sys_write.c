#include <machine/syscall.h>
#include <sys/types.h>
#include "internal_syscall.h"

/* Write to a file.  */
ssize_t
_write(int file, const void *ptr, size_t len)
{
  return syscall_errno (SYS_write, file, ptr, len, 0, 0, 0);
}
