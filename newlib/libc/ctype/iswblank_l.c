/*
FUNCTION
	<<iswblank_l>>---blank wide character test

INDEX
	iswblank_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswblank_l(wint_t <[c]>, locale_t <[locale]>);

DESCRIPTION
<<iswblank_l>> is a function which classifies wide-character values that
are categorized as blank.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<iswblank_l>> returns non-zero if <[c]> is a blank wide character.

PORTABILITY
<<iswblank_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
iswblank_l (wint_t c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return iswblank (c);
}
