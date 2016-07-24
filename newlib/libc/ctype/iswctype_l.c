/*
FUNCTION
	<<iswctype_l>>---extensible wide-character test

INDEX
	iswctype_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswctype_l(wint_t <[c]>, wctype_t <[desc]>, locale_t <[locale]>);

DESCRIPTION
<<iswctype_l>> is a function which classifies wide-character values using the
wide-character test specified by <[desc]>.

if <[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

RETURNS
<<iswctype_l>> returns non-zero if and only if <[c]> matches the test specified by <[desc]>.
If <[desc]> is unknown, zero is returned.

PORTABILITY
<<iswctype_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>

int
iswctype_l (wint_t c, wctype_t desc, struct __locale_t *locale)
{
  /* We're using a locale-independent representation of upper/lower case
     based on Unicode data.  Thus, the locale doesn't matter. */
  return iswctype (c, desc);
}
