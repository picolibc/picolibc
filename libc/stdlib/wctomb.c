/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
/*
FUNCTION
<<wctomb>>---minimal wide char to multibyte converter

INDEX
        wctomb

SYNOPSIS
        #include <stdlib.h>
        int wctomb(char *<[s]>, wchar_t <[wchar]>);

DESCRIPTION
When __MB_CAPABLE is not defined, this is a minimal ANSI-conforming
implementation of <<wctomb>>.  The
only ``wide characters'' recognized are single bytes,
and they are ``converted'' to themselves.

When __MB_CAPABLE is defined, this routine calls <<_wctomb_r>> to perform
the conversion, passing a state variable to allow state dependent
decoding.  The result is based on the locale setting which may
be restricted to a defined set of locales.

Each call to <<wctomb>> modifies <<*<[s]>>> unless <[s]> is a null
pointer or __MB_CAPABLE is defined and <[wchar]> is invalid.

RETURNS
This implementation of <<wctomb>> returns <<0>> if
<[s]> is <<NULL>>; it returns <<-1>> if __MB_CAPABLE is enabled
and the wchar is not a valid multi-byte character, it returns <<1>>
if __MB_CAPABLE is not defined or the wchar is in reality a single
byte character, otherwise it returns the number of bytes in the
multi-byte character.

PORTABILITY
<<wctomb>> is required in the ANSI C standard.  However, the precise
effects vary with the locale.

<<wctomb>> requires no supporting OS subroutines.
*/

#include <stdlib.h>
#include <errno.h>
#include "local.h"

#ifdef __MB_CAPABLE
static mbstate_t _mbtowc_state;
#define ps &_mbtowc_state
#else
#define ps NULL
#endif

int
wctomb(char *s, wchar_t wchar)
{
    int retval;

    retval = __WCTOMB(s, wchar, ps);
    if (retval == -1) {
#ifdef __MB_CAPABLE
        _mbtowc_state.__count = 0;
#endif
        errno = EILSEQ;
    }
    return retval;
}
