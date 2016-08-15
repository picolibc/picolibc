/*
FUNCTION
	<<iscntrl>>, <<iscntrl_l>>---control character predicate

INDEX
	iscntrl

INDEX
	iscntrl_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int iscntrl(int <[c]>);

	#include <ctype.h>
	int iscntrl_l(int <[c]>, locale_t <[locale]>);

TRAD_SYNOPSIS
	#include <ctype.h>
	int iscntrl(<[c]>);

DESCRIPTION
<<iscntrl>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero for control characters, and 0 
for other characters.  It is defined only if <[c]> is representable as an
unsigned char or if <[c]> is EOF.

<<iscntrl_l>> is like <<iscntrl>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef iscntrl>>' or `<<#undef iscntrl_l>>'.

RETURNS
<<iscntrl>>, <<iscntrl_l>> return non-zero if <[c]> is a delete character
or ordinary control character.

PORTABILITY
<<iscntrl>> is ANSI C.
<<iscntrl_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>



#undef iscntrl
int
_DEFUN(iscntrl,(c),int c)
{
	return(__CTYPE_PTR[c+1] & _C);
}
