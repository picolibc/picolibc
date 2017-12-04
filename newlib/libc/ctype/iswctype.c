/*
FUNCTION
	<<iswctype>>, <<iswctype_l>>---extensible wide-character test

INDEX
	iswctype

INDEX
	iswctype_l

SYNOPSIS
	#include <wctype.h>
	int iswctype(wint_t <[c]>, wctype_t <[desc]>);

	#include <wctype.h>
	int iswctype_l(wint_t <[c]>, wctype_t <[desc]>, locale_t <[locale]>);

DESCRIPTION
<<iswctype>> is a function which classifies wide-character values using the
wide-character test specified by <[desc]>.

<<iswctype_l>> is like <<iswctype>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

RETURNS
<<iswctype>>, <<iswctype_l>> return non-zero if and only if <[c]> matches
the test specified by <[desc]>.  If <[desc]> is unknown, zero is returned.

PORTABILITY
<<iswctype>> is C99.
<<iswctype_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <wctype.h>
#include "local.h"

int
iswctype (wint_t c, wctype_t desc)
{
  switch (desc)
    {
    case WC_ALNUM:
      return iswalnum (c);
    case WC_ALPHA:
      return iswalpha (c);
    case WC_BLANK:
      return iswblank (c);
    case WC_CNTRL:
      return iswcntrl (c);
    case WC_DIGIT:
      return iswdigit (c);
    case WC_GRAPH:
      return iswgraph (c);
    case WC_LOWER:
      return iswlower (c);
    case WC_PRINT:
      return iswprint (c);
    case WC_PUNCT:
      return iswpunct (c);
    case WC_SPACE:
      return iswspace (c);
    case WC_UPPER:
      return iswupper (c);
    case WC_XDIGIT:
      return iswxdigit (c);
    default:
      return 0; /* eliminate warning */
    }

  /* otherwise unknown */
  return 0;
}
