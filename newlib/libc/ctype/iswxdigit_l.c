/*
FUNCTION
	<<iswxdigit_l>>---hexadecimal digit wide character test

INDEX
	iswxdigit_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswxdigit_l(wint_t <[c]>, locale_t <[locale]>);

DESCRIPTION
<<iswxdigit_l>> is a function which classifies wide character values that
are hexadecimal digits.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<iswxdigit_l>> returns non-zero if <[c]> is a hexadecimal digit wide character.

PORTABILITY
<<iswxdigit_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
iswxdigit_l (wint_t c, struct __locale_t *locale)
{
  return ((c >= (wint_t)'0' && c <= (wint_t)'9') ||
	  (c >= (wint_t)'a' && c <= (wint_t)'f') ||
	  (c >= (wint_t)'A' && c <= (wint_t)'F'));
}

