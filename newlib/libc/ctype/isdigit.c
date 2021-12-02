/*
FUNCTION
	<<isdigit>>, <<isdigit_l>>---decimal digit predicate

INDEX
	isdigit

INDEX
	isdigit_l

SYNOPSIS
	#include <ctype.h>
	int isdigit(int <[c]>);

	#include <ctype.h>
	int isdigit_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isdigit>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero for decimal digits, and 0 for
other characters.  It is defined only if <[c]> is representable as an
unsigned char or if <[c]> is EOF.

<<isdigit_l>> is like <<isdigit>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isdigit>>' or `<<#undef isdigit_l>>'.

RETURNS
<<isdigit>>, <<isdigit_l>> return non-zero if <[c]> is a decimal digit
(<<0>>--<<9>>).

PORTABILITY
<<isdigit>> is ANSI C.
<<isdigit_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>


#undef isdigit
int
isdigit (int c)
{
	return(__CTYPE_PTR[c+1] & _N);
}
