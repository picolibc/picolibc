#include "newlib.h"

#ifdef _ARM_V7M

/* Called when a hardware fault occurs.  Users can replace this
   function.  */ 

void 
_fault_isr() 
{ 
  /* Sit an endless loop so that the user can analyze the situation
     from the debugger.  */
  while (1);
}

#endif
