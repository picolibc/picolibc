/*
FUNCTION
	<<iswxdigit>>, <<iswxdigit_l>>---hexadecimal digit wide character test

INDEX
	iswxdigit

INDEX
	iswxdigit_l

SYNOPSIS
	#include <wctype.h>
	int iswxdigit(wint_t <[c]>);

	#include <wctype.h>
	int iswxdigit_l(wint_t <[c]>, locale_t <[locale]>);

DESCRIPTION
<<iswxdigit>> is a function which classifies wide character values that
are hexadecimal digits.

<<iswxdigit_l>> is like <<iswxdigit>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

RETURNS
<<iswxdigit>>, <<iswxdigit_l>> return non-zero if <[c]> is a hexadecimal digit wide character.

PORTABILITY
<<iswxdigit>> is C99.
<<iswxdigit_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
iswxdigit (wint_t c)
{
  return ((c >= (wint_t)'0' && c <= (wint_t)'9') ||
	  (c >= (wint_t)'a' && c <= (wint_t)'f') ||
	  (c >= (wint_t)'A' && c <= (wint_t)'F'));
}
