
/*
FUNCTION
<<islower>>---lowercase character predicate

INDEX
islower

ANSI_SYNOPSIS
#include <ctype.h>
int islower(int <[c]>);

TRAD_SYNOPSIS
#include <ctype.h>
int islower(<[c]>);

DESCRIPTION
<<islower>> is a macro which classifies ASCII integer values by table
lookup.  It is a predicate returning non-zero for minuscules
(lowercase alphabetic characters), and 0 for other characters.
It is defined only if <[c]> is representable as an unsigned char or if
<[c]> is EOF.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef islower>>'.

RETURNS
<<islower>> returns non-zero if <[c]> is a lowercase letter (<<a>>--<<z>>).

PORTABILITY
<<islower>> is ANSI C.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <ctype.h>

#undef islower
int
_DEFUN(islower,(c),int c)
{
	return ((__ctype_ptr__[c+1] & (_U|_L)) == _L);
}

