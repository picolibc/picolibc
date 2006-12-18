#include <stdio.h>

#include "c99ppe.h"

int
_DEFUN (puts, (s),
	char _CONST * s)
{

  /* The return value gets written over s
   */
  send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_PUTS, &s);

  return (int)s;
}

