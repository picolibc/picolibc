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

#include <math.h>
#include <float.h>
#include "local.h"

/* On platforms where long double is as wide as double.  */
#if defined(_LDBL_EQ_DBL)
long double
frexpl (long double x, int *eptr)
{
  return frexp(x, eptr);
}
#else  /* !_DBL_EQ_DBL */
# if (LDBL_MANT_DIG == 53) /* 64-bit long double */
static const double scale = 0x1p54;

union ldbl {
  long double x;
  struct {
#  ifdef __IEEE_LITTLE_ENDIAN /* for Intel CPU */
    __uint32_t fracl;
    __uint32_t frach:20;
    __uint32_t exp:11;
    __uint32_t sign:1;
#  endif
#  ifdef __IEEE_BIG_ENDIAN
    __uint32_t sign:1;
    __uint32_t exp:11;
    __uint32_t frach:20;
#   ifndef ___IEEE_BYTES_LITTLE_ENDIAN
#   else /* ARMEL without __VFP_FP__ */
    __uint32_t frach:20;
    __uint32_t exp:11;
    __uint32_t sign:1;
#   endif
    __uint32_t fracl;
#  endif
  } u32;
};
# elif (LDBL_MANT_DIG == 64) /* 80-bit long double */
static const double scale = 0x1p65;

union ldbl {
  long double x;
  struct {
#  ifdef __IEEE_LITTLE_ENDIAN /* for Intel CPU */
    __uint32_t fracl;
    __uint32_t frach;
    __uint32_t exp:15;
    __uint32_t sign:1;
    __uint32_t pad:16;
#  endif
#  ifdef __IEEE_BIG_ENDIAN
#   ifndef ___IEEE_BYTES_LITTLE_ENDIAN /* for m86k */
    __uint32_t sign:1;
    __uint32_t exp:15;
    __uint32_t pad:16;
#   else /* ARM FPA10 math copprocessor */
    __uint32_t exp:15;
    __uint32_t pad:16;
    __uint32_t sign:1;
#   endif
    __uint32_t frach;
    __uint32_t fracl;
#  endif
  } u32;
};
# elif (LDBL_MANT_DIG == 113) /* 128-bit long double */
static const double scale = 0x1p114;

union ldbl {
  long double x;
  struct {
#  ifdef __IEEE_LITTLE_ENDIAN
    __uint32_t fracl;
    __uint32_t fraclm;
    __uint32_t frachm;
    __uint32_t frach:16;
    __uint32_t exp:15;
    __uint32_t sign:1;
#  endif
#  ifdef __IEEE_BIG_ENDIAN
#   ifndef ___IEEE_BYTES_LITTLE_ENDIAN
    __uint32_t sign:1;
    __uint32_t exp:15;
    __uint32_t frach:16;
#   else /* ARMEL without __VFP_FP__ */
    __uint32_t frach:16;
    __uint32_t exp:15;
    __uint32_t sign:1;
#   endif
    __uint32_t frachm;
    __uint32_t fraclm;
    __uint32_t fracl;
#  endif
  } u32;
};
# else
#  error Unsupported long double format.
# endif

static const int scale_exp = LDBL_MANT_DIG + 1;

long double
frexpl (long double x, int *eptr)
{
  union ldbl u;
  u.x = x;
  int e = u.u32.exp;
  *eptr = 0;
  if (e == (LDBL_MAX_EXP*2 - 1) || x == 0)
    return x; /* inf,nan,0 */
  if (e == 0) /* subnormal */
    {
      u.x *= scale;
      e = u.u32.exp;
      *eptr -= scale_exp;
    }
  *eptr += e - (LDBL_MAX_EXP - 2);
  u.u32.exp = LDBL_MAX_EXP - 2; /* -1 */
  return u.x;
}
#endif /* !_LDBL_EQ_DBL */
