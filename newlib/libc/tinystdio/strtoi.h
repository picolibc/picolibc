/* Copyright (c) 2002,2004,2005 Joerg Wunsch
   Copyright (c) 2008  Dmitry Xmelkov
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdio_private.h"

#if defined(_HAVE_BUILTIN_MUL_OVERFLOW) && defined(_HAVE_BUILTIN_ADD_OVERFLOW) && !defined(strtoi_signed)
#define USE_OVERFLOW
#endif

strtoi_type
strtoi(const char *__restrict nptr, char **__restrict endptr, int ibase)
{
    unsigned int base = ibase;

    /* Check for invalid base value */
    if (base > 36 || base == 1) {
        errno = EINVAL;
        if (endptr)
            *endptr = (char *) nptr;
        return 0;
    }

#define FLAG_NEG        0x1     /* Negative. Must be 1 for ucutoff below */
#define FLAG_OFLOW      0x2     /* Value overflow */

    const unsigned char *s = (const unsigned char *) nptr;
    strtoi_type val = 0;
    unsigned char flags = 0;
    unsigned int i;

    /* Skip leading spaces */
    do {
        i = *s++;
    } while (isspace(i));

    /* Parse a leading sign */
    switch (i) {
    case '-':
        flags = FLAG_NEG;
	__PICOLIBC_FALLTHROUGH;
    case '+':
        i = *s++;
    }

    /* Leading '0' digit -- check for base indication */
    if (i == '0') {
        if (TOLOWER(*s) == 'x' && ((base | 16) == 16)) {
            base = 16;
            /* Parsed the '0' */
            nptr = (const char *) s;
            i = s[1];
            s += 2;
	} else if (base == 0) {
            base = 8;
        }
    } else if (base == 0) {
        base = 10;
    }

#ifndef USE_OVERFLOW
    /* Compute values used to detect overflow. */
#ifdef strtoi_signed
    /* works because strtoi_min = (strtoi_type) ((strtoi_utype) strtoi_max + 1) */
    strtoi_utype ucutoff = (strtoi_utype) strtoi_max + flags;
    strtoi_type cutoff = ucutoff / base;
    unsigned int cutlim = ucutoff % base;
#else
    strtoi_type cutoff = strtoi_max / base;
    unsigned int cutlim = strtoi_max % base;
#endif
#endif

    for(;;) {
        i = digit_to_val(i);
        /* detect invalid char */
        if (i >= base)
            break;

        /* Add the new digit, checking for overflow */
#ifdef USE_OVERFLOW
        /*
         * This isn't used for signed values as it's tricky and
         * generates larger code. Yes, it avoids doing the divmod
         * above, but we'll assume an app doing math with signed
         * values will probably end up doing a divide somewhere
         */
        if (__builtin_mul_overflow(val, (strtoi_type) base, &val) ||
            __builtin_add_overflow(val, (strtoi_type) i, &val))
        {
            flags |= FLAG_OFLOW;
        }
#else
        if (val > cutoff || (val == cutoff && i > cutlim))
            flags |= FLAG_OFLOW;
        val = val * (strtoi_type) base + (strtoi_type) i;
#endif
        /* Parsed another digit */
        nptr = (const char *) s;
        i = *s++;
    }

    /* Mark the end of the parsed region */
    if (endptr != NULL)
        *endptr = (char *) nptr;

    if (flags & FLAG_NEG)
        val = -val;

    if (flags & FLAG_OFLOW) {
#ifdef strtoi_signed
        val = (strtoi_type) ucutoff;
#else
        val = strtoi_max;
#endif
        errno = ERANGE;
    }

    return val;
}
