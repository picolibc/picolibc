#include <machine/syscall.h>
#include <sys/types.h>
#include "internal_syscall.h"

/* Increase program data space. As malloc and related functions depend
   on this, it is useful to have a working implementation. The following
   is suggested by the newlib docs and suffices for a standalone
   system.  */
void *
_sbrk(ptrdiff_t incr)
{
  static unsigned long heap_end;

  if (heap_end == 0)
    {
      long brk = syscall_errno (SYS_brk, 0, 0, 0, 0, 0, 0);
      if (brk == -1)
	return (void *)-1;
      heap_end = brk;
    }

  if (syscall_errno (SYS_brk, heap_end + incr, 0, 0, 0, 0, 0) != heap_end + incr)
    return (void *)-1;

  heap_end += incr;
  return (void *)(heap_end - incr);
}
