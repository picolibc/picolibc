/*
Copyright (c) 2002 Red Hat Incorporated.
All rights reserved.
Modified (m) 2017 Thomas Wolff to refer to generated Unicode data tables.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

The name of Red Hat Incorporated may not be used to endorse
or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS   
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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

#define _DEFAULT_SOURCE
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
