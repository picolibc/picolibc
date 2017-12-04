/* The signgam variable is stored in the reentrancy structure.  This
   function returns its address for use by the macro signgam defined in
   math.h.  */

#include <math.h>
#include <reent.h>

#ifndef _REENT_ONLY

int *
__signgam (void)
{
  return &_REENT_SIGNGAM(_REENT);
}

#endif
