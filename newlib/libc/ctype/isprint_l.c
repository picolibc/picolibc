
/*
FUNCTION
	<<isprint_l>>, <<isgraph_l>>---printable character predicates

INDEX
	isprint_l
INDEX
	isgraph_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int isprint_l(int <[c]>, locale_t <[locale]>);
	int isgraph_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isprint_l>> is a macro which classifies ASCII integer values by table
lookup in locale <[locale]>.  It is a predicate returning non-zero for
printable characters, and 0 for other character arguments.  It is defined
only if <[c]> is representable as an unsigned char or if <[c]> is EOF.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining either macro using `<<#undef isprint_l>>' or `<<#undef isgraph_l>>'.

RETURNS
<<isprint_l>> returns non-zero if <[c]> is a printing character,
(<<0x20>>--<<0x7E>>).
<<isgraph_l>> behaves identically to <<isprint_l>>, except that the space
character (<<0x20>>) is excluded.

PORTABILITY
<<isprint_l>> and <<isgraph_l>> are POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>

#undef isgraph_l

int
isgraph_l (int c, struct __locale_t *locale)
{
  return __locale_ctype_ptr_l (locale)[c+1] & (_P|_U|_L|_N);
}

#undef isprint_l

int
isprint_l (int c, struct __locale_t *locale)
{
  return __locale_ctype_ptr_l (locale)[c+1] & (_P|_U|_L|_N|_B);
}
