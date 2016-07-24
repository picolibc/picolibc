/*
FUNCTION
	<<toascii_l>>---force integers to ASCII range

INDEX
	toascii_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int toascii_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<toascii_l>> is a macro which coerces integers to the ASCII range (0--127) by zeroing any higher-order bits.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining this macro using `<<#undef toascii_l>>'.

RETURNS
<<toascii_l>> returns integers between 0 and 127.

PORTABILITY
<<toascii_l>> is a GNU extension.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>

#undef toascii_l

int
toascii_l (int c, struct __locale_t *locale)
{
  return c & 0177;
}
