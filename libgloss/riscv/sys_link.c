#include <machine/syscall.h>
#include "internal_syscall.h"

/* Establish a new name for an existing file.  */
int _link(const char *old_name, const char *new_name)
{
  return syscall_errno (SYS_link, old_name, new_name, 0, 0, 0, 0);
}
