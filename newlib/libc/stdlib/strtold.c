/*
(C) Copyright IBM Corp. 2009

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of IBM nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

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

#define _GNU_SOURCE
#include <stdlib.h>
#include "local.h"
#include "mprec.h"
#undef FLT_ROUNDS

#if defined(_HAVE_LONG_DOUBLE) && __LDBL_MANT_DIG__ == 64 && !defined(_LDBL_EQ_DBL)

/* Intel MCU has no x87 floating point unit */
#if (defined (__x86_64__) || defined (__i386__)) && !defined (__iamcu__)
static const int map[] = {
        1,      /* round to nearest */
        3,      /* round to zero */
        2,      /* round to negative infinity */
        0       /* round to positive infinity */
};

int
__flt_rounds(void)
{
        int x;

        /* Assume that the x87 and the SSE unit agree on the rounding mode. */
        __asm__("fnstcw %0" : "=m" (x));
        return (map[(x >> 10) & 0x03]);
}
#define FLT_ROUNDS __flt_rounds()
#else
#define FLT_ROUNDS 0
#endif

long double
strtold_l (const char *__restrict s00, char **__restrict se, locale_t loc)
{
  long double result;

  _strtorx_l (s00, se, FLT_ROUNDS, &result, loc);
  return result;
}

long double
strtold (const char *__restrict s00, char **__restrict se)
{
  long double result;

  _strtorx_l (s00, se, FLT_ROUNDS, &result, __get_current_locale ());
  return result;
}

#endif /* _HAVE_LONG_DOUBLE */

