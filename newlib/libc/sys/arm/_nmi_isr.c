#include "newlib.h"

/* Called when a non-maskable interrupt occurs.  Users can replace this
   function.  */ 

void
_nmi_isr() 
{ 
  /* Sit an endless loop so that the user can analyze the situation
     from the debugger.  */
  while (1);
}
