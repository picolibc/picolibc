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
	<<iswcntrl>>, <<iswcntrl_l>>---control wide character test

INDEX
	iswcntrl

INDEX
	iswcntrl_l

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswcntrl(wint_t <[c]>);

	#include <wctype.h>
	int iswcntrl_l(wint_t <[c]>, locale_t <[locale]>);

TRAD_SYNOPSIS
	#include <wctype.h>
	int iswcntrl(<[c]>)
	wint_t <[c]>;

DESCRIPTION
<<iswcntrl>> is a function which classifies wide-character values that
are categorized as control characters.

<<iswcntrl_l>> is like <<iswcntrl>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

RETURNS
<<iswcntrl>>, <<iswcntrl_l>> return non-zero if <[c]> is a control wide character.

PORTABILITY
<<iswcntrl>> is C99.
<<iswcntrl_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <newlib.h>
#include <wctype.h>
#include <ctype.h>
#include <string.h>
#include "local.h"

int
_DEFUN(iswcntrl,(c), wint_t c)
{
#ifdef _MB_CAPABLE
  c = _jp2uc (c);

  /* Based on Unicode 5.2.  All characters from general category "Cc", "Zl",
     and "Zp".  */
  return ((c >= 0x0000 && c <= 0x001f) || 
	  (c >= 0x007f && c <= 0x009f) ||
	  c == 0x2028 || c == 0x2029);
#else
  return (c < 0x100 ? iscntrl (c) : 0);
#endif /* _MB_CAPABLE */
}
