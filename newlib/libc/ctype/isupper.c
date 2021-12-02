/*
FUNCTION
	<<isupper>>, <<isupper_l>>---uppercase character predicate

INDEX
	isupper

INDEX
	isupper_l

SYNOPSIS
	#include <ctype.h>
	int isupper(int <[c]>);

	#include <ctype.h>
	int isupper_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isupper>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero for uppercase letters
(<<A>>--<<Z>>), and 0 for other characters.

<<isupper_l>> is like <<isupper>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isupper>>' or `<<#undef isupper_l>>'.

RETURNS
<<isupper>>, <<isupper_l>> return non-zero if <[c]> is an uppercase letter.

PORTABILITY
<<isupper>> is ANSI C.
<<isupper_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <ctype.h>

#undef isupper
int
isupper (int c)
{
	return ((__CTYPE_PTR[c+1] & (_U|_L)) == _U);
}
