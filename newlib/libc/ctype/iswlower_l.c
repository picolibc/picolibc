
/*
FUNCTION
	<<iswlower_l>>---lowercase wide character test

INDEX
	iswlower_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswlower_l(wint_t <[c]>, locale_t <[locale]>);

DESCRIPTION
<<iswlower_l>> is a function which classifies wide-character values that
have uppercase translations.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<iswlower_l>> returns non-zero if <[c]> is a lowercase wide character.

PORTABILITY
<<iswlower_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
iswlower_l (wint_t c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return (towupper (c) != c);
}
