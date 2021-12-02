/*
FUNCTION
	<<isprint>>, <<isgraph>>, <<isprint_l>>, <<isgraph_l>>---printable character predicates

INDEX
	isprint

INDEX
	isgraph

INDEX
	isprint_l

INDEX
	isgraph_l

SYNOPSIS
	#include <ctype.h>
	int isprint(int <[c]>);
	int isgraph(int <[c]>);

	#include <ctype.h>
	int isprint_l(int <[c]>, locale_t <[locale]>);
	int isgraph_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isprint>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero for printable characters,
and 0 for other character arguments.  It is defined only if <[c]> is
representable as an unsigned char or if <[c]> is EOF.

<<isgraph>> behaves identically to <<isprint>>, except that space characters
are excluded.

<<isprint_l>>, <<isgraph_l>> are like <<isprint>>, <<isgraph>> but perform
the check based on the locale specified by the locale object locale.  If
<[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining either macro using `<<#undef isprint>>' or `<<#undef isgraph>>',
or `<<#undef isprint_l>>' or `<<#undef isgraph_l>>'.

RETURNS
<<isprint>>, <<isprint_l>> return non-zero if <[c]> is a printing character.
<<isgraph>>, <<isgraph_l>> return non-zero if <[c]> is a printing character
except spaces.

PORTABILITY
<<isprint>> and <<isgraph>> are ANSI C.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>

#undef isgraph
int
isgraph (int c)
{
	return(__CTYPE_PTR[c+1] & (_P|_U|_L|_N));
}


#undef isprint
int
isprint (int c)
{
	return(__CTYPE_PTR[c+1] & (_P|_U|_L|_N|_B));
}
