/*
FUNCTION
	<<isxdigit>>, <<isxdigit_l>>---hexadecimal digit predicate

INDEX
	isxdigit

INDEX
	isxdigit_l

SYNOPSIS
	#include <ctype.h>
	int isxdigit(int <[c]>);

	#include <ctype.h>
	int isxdigit_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isxdigit>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero for hexadecimal digits,
and <<0>> for other characters.  It is defined only if <[c]> is
representable as an unsigned char or if <[c]> is EOF.

<<isxdigit_l>> is like <<isxdigit>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isxdigit>>' or `<<#undef isxdigit_l>>'.

RETURNS
<<isxdigit>>, <<isxdigit_l>> return non-zero if <[c]> is a hexadecimal digit
(<<0>>--<<9>>, <<a>>--<<f>>, or <<A>>--<<F>>).

PORTABILITY
<<isxdigit>> is ANSI C.
<<isxdigit_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <ctype.h>


#undef isxdigit
int
isxdigit (int c)
{
	return(__CTYPE_PTR[c+1] & ((_X)|(_N)));
}
