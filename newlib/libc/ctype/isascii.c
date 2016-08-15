/*
FUNCTION
	<<isascii>>, <<isascii_l>>---ASCII character predicate

INDEX
	isascii

INDEX
	isascii_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int isascii(int <[c]>);

	#include <ctype.h>
	int isascii_l(int <[c]>, locale_t <[locale]>);

TRAD_SYNOPSIS
	#include <ctype.h>
	int isascii(<[c]>);

DESCRIPTION
<<isascii>> is a macro which returns non-zero when <[c]> is an ASCII
character, and 0 otherwise.  It is defined for all integer values.

<<isascii_l>> is like <<isascii>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isascii>>' or `<<#undef isascii_l>>'.

RETURNS
<<isascii>>, <<isascii_l>> return non-zero if the low order byte of <[c]>
is in the range 0 to 127 (<<0x00>>--<<0x7F>>).

PORTABILITY
<<isascii>> is ANSI C.
<<isascii_l>> is a GNU extension.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <ctype.h>



#undef isascii

int 
_DEFUN(isascii,(c),int c)
{
	return c >= 0 && c< 128;
}
