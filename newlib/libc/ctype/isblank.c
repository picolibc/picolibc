
/*
FUNCTION
	<<isblank>>---blank character predicate

INDEX
	isblank

ANSI_SYNOPSIS
	#include <ctype.h>
	int isblank(int <[c]>);

TRAD_SYNOPSIS
	#include <ctype.h>
	int isblank(<[c]>);

DESCRIPTION
<<isblank>> is a function which classifies ASCII integer values by table
lookup.  It is a predicate returning non-zero for blank characters, and 0
for other characters.  It is defined only if <[c]> is representable as an
unsigned char or if <[c]> is EOF.

RETURNS
<<isblank>> returns non-zero if <[c]> is a blank character.

PORTABILITY
<<isblank>> is C99.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>



#undef isblank
int
_DEFUN(isblank,(c),int c)
{
	return ((__ctype_ptr__[c+1] & _B) || (c == '\t'));
}
