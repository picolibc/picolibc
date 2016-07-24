/*
FUNCTION
	<<towupper_l>>---translate wide characters to uppercase

INDEX
	towupper_l

ANSI_SYNOPSIS
	#include <wctype.h>
	wint_t towupper_l(wint_t <[c]>, locale_t <[locale]>);

DESCRIPTION
<<towupper_l>> is a function which converts lowercase wide characters to
uppercase, leaving all other characters unchanged.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<towupper_l>> returns the uppercase equivalent of <[c]> when it is a
lowercase wide character, otherwise, it returns the input character.

PORTABILITY
<<towupper_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <wctype.h>

wint_t
towupper_l (wint_t c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return towupper (c);
}
