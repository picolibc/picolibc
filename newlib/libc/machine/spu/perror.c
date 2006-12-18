#include <stdio.h>

#include "c99ppe.h"

void
_DEFUN (perror, (s),
	_CONST char *s)

{
  send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_PERROR, &s);

  return;
}

