/* The getdate_err variable is stored in the reentrancy structure.  This
   function returns its address for use by the getdate_err macro defined in
   time.h.  */

#include <errno.h>
#include <reent.h>

#ifndef _REENT_ONLY

NEWLIB_THREAD_LOCAL int getdate_err;

#endif
