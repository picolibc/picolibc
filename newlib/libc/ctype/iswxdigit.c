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
  return (c >= (wint_t)'0' && c <= (wint_t)'9')
      || (c >= (wint_t)'a' && c <= (wint_t)'f')
      || (c >= (wint_t)'A' && c <= (wint_t)'F');
}
