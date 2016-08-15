/*
FUNCTION
	<<ispunct>>, <<ispunct_l>>---punctuation character predicate

INDEX
	ispunct

INDEX
	ispunct_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int ispunct(int <[c]>);

	#include <ctype.h>
	int ispunct_l(int <[c]>, locale_t <[locale]>);

TRAD_SYNOPSIS
	#include <ctype.h>
	int ispunct(<[c]>);

DESCRIPTION
<<ispunct>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero for printable
punctuation characters, and 0 for other characters.  It is defined only
if <[c]> is representable as an unsigned char or if <[c]> is EOF.

<<ispunct_l>> is like <<ispunct>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef ispunct>>' or `<<#undef ispunct_l>>'.

RETURNS
<<ispunct>>, <<ispunct_l>> return non-zero if <[c]> is a printable
punctuation character.

PORTABILITY
<<ispunct>> is ANSI C.
<<ispunct_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>


#undef ispunct
int
_DEFUN(ispunct,(c),int c)
{
	return(__CTYPE_PTR[c+1] & _P);
}
