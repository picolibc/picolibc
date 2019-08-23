/* The signgam variable is stored in the reentrancy structure.  This
   function returns its address for use by the macro signgam defined in
   math.h.  */

#include <math.h>
#include <reent.h>

#ifndef _REENT_ONLY
NEWLIB_THREAD_LOCAL int signgam;
#endif
