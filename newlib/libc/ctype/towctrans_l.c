/*
FUNCTION
	<<towctrans_l>>---extensible wide-character translation

INDEX
	towctrans_l

ANSI_SYNOPSIS
	#include <wctype.h>
	wint_t towctrans_l(wint_t <[c]>, wctrans_t <[w]>, locale_t <[locale]>);

DESCRIPTION
<<towctrans_l>> is a function which converts wide characters based on
a specified translation type <[w]>.  If the translation type is
invalid or cannot be applied to the current character, no change
to the character is made.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<towctrans_l>> returns the translated equivalent of <[c]> when it is a
valid for the given translation, otherwise, it returns the input character.
When the translation type is invalid, <<errno>> is set <<EINVAL>>.

PORTABILITY
<<towctrans_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

wint_t
towctrans_l (wint_t c, wctrans_t w, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return towctrans (c, w);
}
