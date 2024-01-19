/*
Copyright (c) 1989 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
/*
FUNCTION
	<<tolower>>, <<tolower_l>>---translate characters to lowercase

INDEX
	tolower

INDEX
	tolower_l

INDEX
	_tolower

SYNOPSIS
	#include <ctype.h>
	int tolower(int <[c]>);
	int _tolower(int <[c]>);

	#include <ctype.h>
	int tolower_l(int <[c]>, locale_t <[locale]>);


DESCRIPTION
<<tolower>> is a macro which converts uppercase characters to lowercase,
leaving all other characters unchanged.  It is only defined when
<[c]> is an integer in the range <<EOF>> to <<255>>.

<<tolower_l>> is like <<tolower>> but performs the function based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining this macro using `<<#undef tolower>>' or `<<#undef tolower_l>>'.

<<_tolower>> performs the same conversion as <<tolower>>, but should
only be used when <[c]> is known to be an uppercase character (<<A>>--<<Z>>).

RETURNS
<<tolower>>, <<tolower_l>> return the lowercase equivalent of <[c]> when
<[c]> is an uppercase character, and <[c]> otherwise.

<<_tolower>> returns the lowercase equivalent of <[c]> when it is a
character between <<A>> and <<Z>>.  If <[c]> is not one of these
characters, the behaviour of <<_tolower>> is undefined.

PORTABILITY
<<tolower>> is ANSI C.  <<_tolower>> is not recommended for portable programs.
<<tolower_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/ 

#include <ctype.h>
#if defined (_MB_EXTENDED_CHARSETS_ISO) || defined (_MB_EXTENDED_CHARSETS_WINDOWS)
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#endif

#undef tolower
int
tolower (int c)
{
#if defined (_MB_EXTENDED_CHARSETS_ISO) || defined (_MB_EXTENDED_CHARSETS_WINDOWS)
  if ((unsigned char) c <= 0x7f) 
    return isupper (c) ? c - 'A' + 'a' : c;
  else if (c != EOF && MB_CUR_MAX == 1 && isupper (c))
    {
      char s[MB_LEN_MAX] = { c, '\0' };
      wchar_t wc;
      if (mbtowc (&wc, s, 1) >= 0
	  && wctomb (s, (wchar_t) towlower ((wint_t) wc)) == 1)
	c = (unsigned char) s[0];
    }
  return c;
#else
  return isupper(c) ? (c) - 'A' + 'a' : c;
#endif
}
