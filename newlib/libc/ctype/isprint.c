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
	<<isprint>>, <<isgraph>>, <<isprint_l>>, <<isgraph_l>>---printable character predicates

INDEX
	isprint

INDEX
	isgraph

INDEX
	isprint_l

INDEX
	isgraph_l

SYNOPSIS
	#include <ctype.h>
	int isprint(int <[c]>);
	int isgraph(int <[c]>);

	#include <ctype.h>
	int isprint_l(int <[c]>, locale_t <[locale]>);
	int isgraph_l(int <[c]>, locale_t <[locale]>);

DESCRIPTION
<<isprint>> is a macro which classifies singlebyte charset values by table
lookup.  It is a predicate returning non-zero for printable characters,
and 0 for other character arguments.  It is defined only if <[c]> is
representable as an unsigned char or if <[c]> is EOF.

<<isgraph>> behaves identically to <<isprint>>, except that space characters
are excluded.

<<isprint_l>>, <<isgraph_l>> are like <<isprint>>, <<isgraph>> but perform
the check based on the locale specified by the locale object locale.  If
<[locale]> is LC_GLOBAL_LOCALE or not a valid locale object, the behaviour
is undefined.

You can use a compiled subroutine instead of the macro definition by
undefining either macro using `<<#undef isprint>>' or `<<#undef isgraph>>',
or `<<#undef isprint_l>>' or `<<#undef isgraph_l>>'.

RETURNS
<<isprint>>, <<isprint_l>> return non-zero if <[c]> is a printing character.
<<isgraph>>, <<isgraph_l>> return non-zero if <[c]> is a printing character
except spaces.

PORTABILITY
<<isprint>> and <<isgraph>> are ANSI C.

No supporting OS subroutines are required.
*/

#include <ctype.h>

#undef isprint
int
isprint (int c)
{
#if _PICOLIBC_CTYPE_SMALL
    return ' ' <= c && c <= '~';
#else
    return __CTYPE_PTR[c+1] & (_P|_U|_L|_N|_B);
#endif
}
