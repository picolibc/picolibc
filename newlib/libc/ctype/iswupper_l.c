/*
FUNCTION
	<<iswupper_l>>---uppercase wide character test

INDEX
	iswupper_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswupper_l(wint_t <[c]>, locale_t <[locale]>);

DESCRIPTION
<<iswupper_l>> is a function which classifies wide-character values that
have uppercase translations.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<iswupper_l>> returns non-zero if <[c]> is a uppercase wide character.

PORTABILITY
<<iswupper_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
iswupper_l (wint_t c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return towlower (c) != c;
}
