/*
FUNCTION 
	<<isalnum>>, <<isalnum_l>>---alphanumeric character predicate

INDEX
	isalnum
INDEX
	isalnum_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int isalnum(int <[c]>);

	#include <ctype.h>
	int isalnum_l(int <[c]>, locale_t <[locale]>);

TRAD_SYNOPSIS
	#include <ctype.h>
	int isalnum(<[c]>);


DESCRIPTION
<<isalnum>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero for alphabetic or
numeric ASCII characters, and <<0>> for other arguments.  It is defined
only if <[c]> is representable as an unsigned char or if <[c]> is EOF.

<<isalnum_l>> is like <<isalnum>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isalnum>>' or `<<#undef isalnum_l>>'.

RETURNS
<<isalnum>>,<<isalnum_l>> return non-zero if <[c]> is a letter or a digit.

PORTABILITY
<<isalnum>> is ANSI C.
<<isalnum_l>> is POSIX-1.2008.

No OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>

#undef isalnum

int
_DEFUN(isalnum,(c),int c)
{
	return(__CTYPE_PTR[c+1] & (_U|_L|_N));
}
