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
	<<isalnum>>, <<isalnum_l>>---alphanumeric character predicate

INDEX
	isalnum
INDEX
	isalnum_l

SYNOPSIS
	#include <ctype.h>
	int isalnum(int <[c]>);

	#include <ctype.h>
	int isalnum_l(int <[c]>, locale_t <[locale]>);


DESCRIPTION
<<isalnum>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero for alphabetic or
numeric ASCII characters, and <<0>> for other arguments.  It is defined
only if <[c]> is representable as an unsigned char or if <[c]> is EOF.

<<isalnum_l>> is like <<isalnum>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isalnum>>' or `<<#undef isalnum_l>>'.

RETURNS
<<isalnum>>,<<isalnum_l>> return non-zero if <[c]> is a letter or a digit.

PORTABILITY
<<isalnum>> is ANSI C.
<<isalnum_l>> is POSIX-1.2008.

No OS subroutines are required.
*/

#include <ctype.h>

#undef isalnum
int
isalnum (int c)
{
#if _PICOLIBC_CTYPE_SMALL
    return isalpha(c) || isdigit(c);
#else
    return __CTYPE_PTR[c+1] & (_U|_L|_N);
#endif
}
