/* The errno variable is stored in the reentrancy structure.  This
   function returns its address for use by the macro errno defined in
   errno.h.  */

#include <errno.h>

NEWLIB_THREAD_LOCAL_ERRNO int errno;
