/*
FUNCTION
	<<iswgraph_l>>---graphic wide character test

INDEX
	iswgraph_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswgraph_l(wint_t <[c]>, locale_t <[locale]>);

DESCRIPTION
<<iswgraph_l>> is a function which classifies wide-character values that
are graphic.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<iswgraph_l>> returns non-zero if <[c]> is a graphic wide character.

PORTABILITY
<<iswgraph_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
iswgraph_l (wint_t c, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return iswprint (c) && !iswspace (c);
}
