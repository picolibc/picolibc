/*
FUNCTION
	<<iswalnum>>, <<iswalnum_l>>---alphanumeric wide character test

INDEX
	iswalnum

INDEX
	iswalnum_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswalnum(wint_t <[c]>);

	#include <wctype.h>
	int iswalnum_l(wint_t <[c]>, locale_t <[locale]>);

TRAD_SYNOPSIS
	#include <wctype.h>
	int iswalnum(<[c]>)
	wint_t <[c]>;

DESCRIPTION
<<iswalnum>> is a function which classifies wide-character values that
are alphanumeric.

<<iswalnum_l>> is like <<iswalnum>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

RETURNS
<<iswalnum>>, <<iswalnum_l>> return non-zero if <[c]> is a alphanumeric
wide character.

PORTABILITY
<<iswalnum>> is C99.
<<iswalnum_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
_DEFUN(iswalnum,(c),wint_t c)
{
  return (iswalpha (c) || iswdigit (c));
}
