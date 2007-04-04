#include <stdio.h>

#include "c99ppe.h"

#ifndef _REENT_ONLY

void
_DEFUN (perror, (s),
	_CONST char *s)

{
  CHECK_STD_INIT(_REENT);

  __send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_PERROR, &s);

  return;
}
#endif /* ! _REENT_ONLY */
