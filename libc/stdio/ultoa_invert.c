/* Copyright © 2017 Keith Packard
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
  POSSIBILITY OF SUCH DAMAGE. */

#include "udivmod10.h"

/*
 * Enable fancy divmod when the conversion type is wider than 'long',
 * where binary-to-decimal conversion would otherwise use slow soft
 * division code (e.g. 64-bit division on 32-bit targets) which is
 * often quite large as well.  The _UDIVMOD10_DIVIDE_FREE predicate of
 * udivmod10.h decides between this and sharing the soft division
 * helper, and excludes targets with native 64-bit division.
 */
#if defined(_UDIVMOD10_DIVIDE_FREE) && SIZEOF_ULTOA > __SIZEOF_LONG__

#define FANCY_DIVMOD

static inline ultoa_unsigned_t
udivmod10(ultoa_unsigned_t n, char *rp)
{
#if SIZEOF_ULTOA > 4
    return udivmod10_64(n, rp);
#else
    return udivmod10_32(n, rp);
#endif
}

/*
 * Digit extraction for the bases used by the printf converters: 2
 * (when %b is enabled), 8, 10 and 16. Any other value falls back to
 * division by ten, so callers must only pass those bases.
 */
static inline ultoa_unsigned_t
udivmod(ultoa_unsigned_t val, int base, char *dig)
{
    switch (base) {
#ifdef __IO_PERCENT_B
    case 2:
        *dig = val & 1;
        return val >> 1;
#endif
    case 8:
        *dig = val & 7;
        return val >> 3;
    case 16:
        *dig = val & 15;
        return val >> 4;
    }
    return udivmod10(val, dig);
}

#endif

static __noinline char *
__ultoa_invert(ultoa_unsigned_t val, char *str, int base)
{
    char hex = ('a' - '0' - 10 + 16) - base;

    base &= 31;

    do {
        char v;

#ifdef FANCY_DIVMOD
        val = udivmod(val, base, &v);
#else
        v = val % base;
        val /= base;
#endif
        if (v > 9)
            v += hex;
        v += '0';
        *str++ = v;
    } while (val);
    return str;
}
