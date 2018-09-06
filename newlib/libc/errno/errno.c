/* The errno variable is stored in the reentrancy structure.  This
   function returns its address for use by the macro errno defined in
   errno.h.  */

#include <errno.h>
#include <reent.h>

#ifdef NEWLIB_GLOBAL_ERRNO
int errno;
#else

#ifndef _REENT_ONLY

int *
__errno ()
{
  return &__errno_r(_REENT);
}

#endif
#endif
