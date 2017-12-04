/*
FUNCTION
	<<toascii>>, <<toascii_l>>---force integers to ASCII range

INDEX
	toascii

INDEX
	toascii_l

SYNOPSIS
	#include <ctype.h>
	int toascii(int <[c]>);

	#include <ctype.h>
	int toascii_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<toascii>> is a macro which coerces integers to the ASCII range (0--127) by zeroing any higher-order bits.

<<toascii_l>> is like <<toascii>> but performs the function based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining this macro using `<<#undef toascii>>' or `<<#undef toascii_l>>'.

RETURNS
<<toascii>>, <<toascii_l>> return integers between 0 and 127.

PORTABILITY
<<toascii>> is X/Open, BSD and POSIX-1.2001, but marked obsolete in
POSIX-1.2008.
<<toascii_l>> is a GNU extension.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>
#undef toascii

int
toascii (int c)
{
  return (c)&0177;
}

