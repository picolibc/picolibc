/*
FUNCTION
	<<isblank_l>>---blank character predicate

INDEX
	isblank_l

ANSI_SYNOPSIS
	#include <ctype.h>
	int isblank_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isblank_l>> is a function which classifies ASCII integer values by table
lookup in locale <[locale]>.  It is a predicate returning non-zero for blank
characters, and 0 for other characters.  It is defined only if <[c]> is
representable as an unsigned char or if <[c]> is EOF.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<isblank_l>> returns non-zero if <[c]> is a blank character.

PORTABILITY
<<isblank_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>

#undef isblank_l

int
isblank_l (int c, struct __locale_t *locale)
{
  return (__locale_ctype_ptr_l (locale)[c+1] & _B) || (c == '\t');
}
