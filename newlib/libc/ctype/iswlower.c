/*
FUNCTION
	<<iswlower>>, <<iswlower_l>>---lowercase wide character test

INDEX
	iswlower

INDEX
	iswlower_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswlower(wint_t <[c]>);

	#include <wctype.h>
	int iswlower_l(wint_t <[c]>, locale_t <[locale]>);

TRAD_SYNOPSIS
	#include <wctype.h>
	int iswlower(<[c]>)
	wint_t <[c]>;

DESCRIPTION
<<iswlower>> is a function which classifies wide-character values that
have uppercase translations.

<<iswlower_l>> is like <<iswlower>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

RETURNS
<<iswlower>>, <<iswlower_l>> return non-zero if <[c]> is a lowercase wide character.

PORTABILITY
<<iswlower>> is C99.
<<iswlower_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
_DEFUN(iswlower,(c),wint_t c)
{
	return (towupper (c) != c);
}
