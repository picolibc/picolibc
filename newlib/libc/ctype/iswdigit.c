/*
FUNCTION
	<<iswdigit>>, <<iswdigit_l>>---decimal digit wide character test

INDEX
	iswdigit

INDEX
	iswdigit_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswdigit(wint_t <[c]>);

	#include <wctype.h>
	int iswdigit_l(wint_t <[c]>, locale_t <[locale]>);

TRAD_SYNOPSIS
	#include <wctype.h>
	int iswdigit(<[c]>)
	wint_t <[c]>;

DESCRIPTION
<<iswdigit>> is a function which classifies wide-character values that
are decimal digits.

<<iswdigit_l>> is like <<iswdigit>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

RETURNS
<<iswdigit>>, <<iswdigit_l>> return non-zero if <[c]> is a decimal digit wide character.

PORTABILITY
<<iswdigit>> is C99.
<<iswdigit_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
_DEFUN(iswdigit,(c), wint_t c)
{
  return (c >= (wint_t)'0' && c <= (wint_t)'9');
}
