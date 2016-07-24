/*
FUNCTION
	<<isascii_l>>---ASCII character predicate

INDEX
	isascii_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int isascii_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isascii_l>> is a macro which returns non-zero when <[c]> is an ASCII
character, and 0 otherwise.  It is defined for all integer values.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isascii_l>>'.

RETURNS
<<isascii_l>> returns non-zero if the low order byte of <[c]> is in the range
0 to 127 (<<0x00>>--<<0x7F>>).

PORTABILITY
<<isascii_l>> is a GNU extension.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <ctype.h>

#undef isascii_l

int
isascii_l (int c, struct __locale_t *locale)
{
  return c >= 0 && c < 128;
}
