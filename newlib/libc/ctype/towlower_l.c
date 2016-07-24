/*
FUNCTION
	<<towlower_l>>---translate wide characters to lowercase

INDEX
	towlower_l

ANSI_SYNOPSIS
	#include <wctype.h>
	wint_t towlower_l(wint_t <[c]>, local_t <[locale]>);

DESCRIPTION
<<towlower_l>> is a function which converts uppercase wide characters to
lowercase, leaving all other characters unchanged.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<towlower_l>> returns the lowercase equivalent of <[c]> when it is a
uppercase wide character; otherwise, it returns the input character.

PORTABILITY
<<towlower_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <newlib.h>
#include <wctype.h>
#include "local.h"

wint_t
towlower_l (wint_t c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return towlower (c);
}
