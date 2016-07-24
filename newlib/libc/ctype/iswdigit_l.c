/*
FUNCTION
	<<iswdigit_l>>---decimal digit wide character test

INDEX
	iswdigit_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswdigit_l(wint_t <[c]>, locale_t <[locale]>);

DESCRIPTION
<<iswdigit_l>> is a function which classifies wide-character values that
are decimal digits.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<iswdigit_l>> returns non-zero if <[c]> is a decimal digit wide character.

PORTABILITY
<<iswdigit_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
iswdigit_l (wint_t c, struct __locale_t *locale)
{
  return (c >= (wint_t)'0' && c <= (wint_t)'9');
}
