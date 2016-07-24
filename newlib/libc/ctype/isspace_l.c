/*
FUNCTION
	<<isspace_l>>---whitespace character predicate

INDEX
	isspace_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int isspace_l(int <[c]>, locale_t *<[locale]>);

DESCRIPTION
<<isspace_l>> is a macro which classifies ASCII integer values by table
lookup in locale <[locale]>.  It is a predicate returning non-zero for
whitespace characters, and 0 for other characters.  It is defined only
when <<isascii>>(<[c]>) is true or <[c]> is EOF.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isspace_l>>'.

RETURNS
<<isspace_l>> returns non-zero if <[c]> is a space, tab, carriage return, new
line, vertical tab, or formfeed (<<0x09>>--<<0x0D>>, <<0x20>>).

PORTABILITY
<<isspace_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <ctype.h>

#undef isspace_l

int
isspace_l (int c, struct __locale_t *locale)
{
  return __locale_ctype_ptr_l (locale)[c+1] & _S;
}

