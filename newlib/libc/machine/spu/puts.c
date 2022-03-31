#include <stdio.h>

#include "c99ppe.h"

#ifndef _REENT_ONLY

int
puts (char const * s)
{
  /* The return value gets written over s
   */
  return __send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_PUTS, &s);
}

#endif /* ! _REENT_ONLY */
