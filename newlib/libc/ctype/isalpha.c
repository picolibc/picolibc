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
	<<isalpha>>, <<isalpha_l>>---alphabetic character predicate

INDEX
	isalpha

INDEX
	isalpha_l

SYNOPSIS
	#include <ctype.h>
	int isalpha(int <[c]>);

	#include <ctype.h>
	int isalpha_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isalpha>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero when <[c]> represents an
alphabetic ASCII character, and 0 otherwise.  It is defined only if
<[c]> is representable as an unsigned char or if <[c]> is EOF.

<<isalpha_l>> is like <<isalpha>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isalpha>>' or `<<#undef isalpha_l>>'.

RETURNS
<<isalpha>>, <<isalpha_l>> return non-zero if <[c]> is a letter.

PORTABILITY
<<isalpha>> is ANSI C.
<<isalpha_l>> is POSIX-1.2008.

No supporting OS subroutines are required.
*/

#include <ctype.h>

#undef isalpha
int
isalpha (int c)
{
#if _PICOLIBC_CTYPE_SMALL
    return isupper(c) || islower(c);
#else
    return __CTYPE_PTR[c+1] & (_U|_L);
#endif
}
