/*
FUNCTION
	<<isalpha>>, <<isalpha_l>>---alphabetic character predicate

INDEX
	isalpha

INDEX
	isalpha_l

SYNOPSIS
	#include <ctype.h>
	int isalpha(int <[c]>);

	#include <ctype.h>
	int isalpha_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isalpha>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero when <[c]> represents an
alphabetic ASCII character, and 0 otherwise.  It is defined only if
<[c]> is representable as an unsigned char or if <[c]> is EOF.

<<isalpha_l>> is like <<isalpha>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isalpha>>' or `<<#undef isalpha_l>>'.

RETURNS
<<isalpha>>, <<isalpha_l>> return non-zero if <[c]> is a letter.

PORTABILITY
<<isalpha>> is ANSI C.
<<isalpha_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>

#undef isalpha
int
isalpha (int c)
{
	return(__CTYPE_PTR[c+1] & (_U|_L));
}
