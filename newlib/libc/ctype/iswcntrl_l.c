/*
FUNCTION
	<<iswcntrl_l>>---control wide character test

INDEX
	iswcntrl_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswcntrl_l(wint_t <[c]>, locale_t <[locale]>);

DESCRIPTION
<<iswcntrl_l>> is a function which classifies wide-character values that
are categorized as control characters.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<iswcntrl_l>> returns non-zero if <[c]> is a control wide character.

PORTABILITY
<<iswcntrl_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
iswcntrl_l (wint_t c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return iswcntrl (c);
}
