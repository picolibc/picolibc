#include <stdio.h>

#include "c99ppe.h"

#undef putchar

int
putchar (c)
     int c;
{
  /* c gets overwritten before return */

  send_to_ppe(SPE_C99_SIGNALCODE, SPE_C99_PUTCHAR, &c);

  return c;
}

