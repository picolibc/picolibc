/* Copyright (c) 2002 Red Hat Incorporated.
   All rights reserved.

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
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS   
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
FUNCTION
	<<iswspace>>, <<iswspace_l>>---whitespace wide character test

INDEX
	iswspace

INDEX
	iswspace_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswspace(wint_t <[c]>);

	#include <wctype.h>
	int iswspace_l(wint_t <[c]>, locale_t <[locale]>);

TRAD_SYNOPSIS
	#include <wctype.h>
	int iswspace(<[c]>)
	wint_t <[c]>;

DESCRIPTION
<<iswspace>> is a function which classifies wide-character values that
are categorized as whitespace.

<<iswspace_l>> is like <<iswspace>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

RETURNS
<<iswspace>>, <<iswspace_l>> return non-zero if <[c]> is a whitespace wide character.

PORTABILITY
<<iswspace>> is C99.
<<iswspace_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <newlib.h>
#include <wctype.h>
#include <ctype.h>
#include <string.h>
#include "local.h"

int
_DEFUN(iswspace,(c), wint_t c)
{
#ifdef _MB_CAPABLE
  c = _jp2uc (c);
  /* Based on Unicode 5.2.  Control chars 09-0D, plus all characters
     from general category "Zs", which are not marked as decomposition
     type "noBreak". */
  return ((c >= 0x0009 && c <= 0x000d) || c == 0x0020 ||
	  c == 0x1680 || c == 0x180e ||
	  (c >= 0x2000 && c <= 0x2006) ||
	  (c >= 0x2008 && c <= 0x200a) ||
	  c == 0x2028 || c == 0x2029 ||
	  c == 0x205f || c == 0x3000);
#else
  return (c < 0x100 ? isspace (c) : 0);
#endif /* _MB_CAPABLE */
}
