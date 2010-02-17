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
	<<iswblank>>---blank wide character test

INDEX
	iswblank

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswblank(wint_t <[c]>);

TRAD_SYNOPSIS
	#include <wctype.h>
	int iswblank(<[c]>)
	wint_t <[c]>;

DESCRIPTION
<<iswblank>> is a function which classifies wide-character values that
are categorized as blank.

RETURNS
<<iswblank>> returns non-zero if <[c]> is a blank wide character.

PORTABILITY
<<iswblank>> is C99.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <newlib.h>
#include <wctype.h>
#include <ctype.h>
#include <string.h>
#include "local.h"

int
_DEFUN(iswblank,(c), wint_t c)
{
#ifdef _MB_CAPABLE
  c = _jp2uc (c);
  /* Based on Unicode 5.2.  Control char 09, plus all characters
     from general category "Zs", which are not marked as decomposition
     type "noBreak". */
  return (c == 0x0009 || c == 0x0020 ||
	  c == 0x1680 || c == 0x180e ||
	  (c >= 0x2000 && c <= 0x2006) ||
	  (c >= 0x2008 && c <= 0x200a) ||
	  c == 0x205f || c == 0x3000);
#else
  return (c < 0x100 ? isblank (c) : 0);
#endif /* _MB_CAPABLE */
}

