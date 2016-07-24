/*
FUNCTION
	<<wctrans_l>>---get wide-character translation type

INDEX
	wctrans_l

ANSI_SYNOPSIS
	#include <wctype.h>
	wctrans_t wctrans_l(const char *<[c]>, locale_t <[locale]>);

DESCRIPTION
<<wctrans_l>> is a function which takes a string <[c]> and gives back
the appropriate wctrans_t type value associated with the string,
if one exists.  The following values are guaranteed to be recognized:
"tolower" and "toupper".

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<wctrans_l>> returns 0 and sets <<errno>> to <<EINVAL>> if the
given name is invalid.  Otherwise, it returns a valid non-zero wctrans_t
value.

PORTABILITY
<<wctrans_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

wctrans_t
wctrans_l (const char *c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return wctrans (c);
}
