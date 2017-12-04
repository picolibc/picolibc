#include <_ansi.h>
#include <stdlib.h>
#include <reent.h>
#include <string.h>

/* Nothing in newlib actually *calls* dtoa, they all call _dtoa_r, so this 
   is a safe way of providing it to the user. */
#ifndef _REENT_ONLY

char *
__dtoa (double d,
	int mode,
	int ndigits,
	int *decpt,
	int *sign,
	char **rve)
{
  return _dtoa_r (_REENT, d, mode, ndigits, decpt, sign, rve);
}

#endif
