/*
FUNCTION
	<<iswalnum_l>>---alphanumeric wide character test

INDEX
	iswalnum_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswalnum_l(wint_t <[c]>, locale_t <[locale]>);

DESCRIPTION
<<iswalnum_l>> is a function which classifies wide-character values that
are alphanumeric in locale <[locale]>.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<iswalnum_l>> returns non-zero if <[c]> is a alphanumeric wide character.

PORTABILITY
<<iswalnum_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
iswalnum_l (wint_t c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return iswalpha (c) || iswdigit (c);
}
