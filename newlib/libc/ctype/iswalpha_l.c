/*
FUNCTION
	<<iswalpha_l>>---alphabetic wide character test

INDEX
	iswalpha_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswalpha_l(wint_t <[c]>, locale_t <[locale]>);

DESCRIPTION
<<iswalpha_l>> is a function which classifies wide-character values that
are alphabetic.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<iswalpha_l>> returns non-zero if <[c]> is an alphabetic wide character.

PORTABILITY
<<iswalpha_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
iswalpha_l (wint_t c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return iswalpha (c);
}
