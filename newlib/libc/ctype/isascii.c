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
	<<isascii>>, <<isascii_l>>---ASCII character predicate

INDEX
	isascii

INDEX
	isascii_l

SYNOPSIS
	#include <ctype.h>
	int isascii(int <[c]>);

	#include <ctype.h>
	int isascii_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isascii>> is a macro which returns non-zero when <[c]> is an ASCII
character, and 0 otherwise.  It is defined for all integer values.

<<isascii_l>> is like <<isascii>> but performs the check based on the
locale specified by the locale object locale.  If <[locale]> is
LC_GLOBAL_LOCALE or not a valid locale object, the behaviour is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isascii>>' or `<<#undef isascii_l>>'.

RETURNS
<<isascii>>, <<isascii_l>> return non-zero if the low order byte of <[c]>
is in the range 0 to 127 (<<0x00>>--<<0x7F>>).

PORTABILITY
<<isascii>> is ANSI C.
<<isascii_l>> is a GNU extension.

No supporting OS subroutines are required.
*/
#include <ctype.h>

#undef isascii
int
isascii (int c)
{
    return (unsigned) c <= 0177;
}
