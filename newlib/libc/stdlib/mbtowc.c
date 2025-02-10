/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
/*
FUNCTION
<<mbtowc>>---minimal multibyte to wide char converter

INDEX
	mbtowc

SYNOPSIS
	#include <stdlib.h>
	int mbtowc(wchar_t *restrict <[pwc]>, const char *restrict <[s]>, size_t <[n]>);

DESCRIPTION
When _MB_CAPABLE is not defined, this is a minimal ANSI-conforming 
implementation of <<mbtowc>>.  In this case,
only ``multi-byte character sequences'' recognized are single bytes,
and they are ``converted'' to themselves.
Each call to <<mbtowc>> copies one character from <<*<[s]>>> to
<<*<[pwc]>>>, unless <[s]> is a null pointer.  The argument n
is ignored.

When _MB_CAPABLE is defined, this routine uses a state variable to
allow state dependent decoding.  The result is based on the locale
setting which may be restricted to a defined set of locales.

RETURNS
This implementation of <<mbtowc>> returns <<0>> if
<[s]> is <<NULL>> or is the empty string; 
it returns <<1>> if not _MB_CAPABLE or
the character is a single-byte character; it returns <<-1>>
if n is <<0>> or the multi-byte character is invalid; 
otherwise it returns the number of bytes in the multibyte character.
If the return value is -1, no changes are made to the <<pwc>>
output string.  If the input is the empty string, a wchar_t nul
is placed in the output string and 0 is returned.  If the input
has a length of 0, no changes are made to the <<pwc>> output string.

PORTABILITY
<<mbtowc>> is required in the ANSI C standard.  However, the precise
effects vary with the locale.

<<mbtowc>> requires no supporting OS subroutines.
*/

#include <stdlib.h>
#include <wchar.h>
#include "local.h"

#ifdef _MB_CAPABLE
static mbstate_t _mbtowc_state;
#define ps &_mbtowc_state
#else
#define ps NULL
#endif

int
mbtowc (wchar_t *__restrict pwc,
        const char *__restrict s,
        size_t n)
{
        int retval;

        retval = __MBTOWC (pwc, s, n, ps);
        if (retval < 0) {
#ifdef _MB_CAPABLE
                _mbtowc_state.__count = 0;
#endif
                errno = EILSEQ;
        }
        return retval;
}
