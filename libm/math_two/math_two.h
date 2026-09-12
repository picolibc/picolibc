/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MATH_TWO_H_
#define _MATH_TWO_H_

#include "fdlibm.h"
#include "../ld/math_ld.h"
#include <math.h>
#include <inttypes.h>

/* Figure out mapping between C types and floating point formats */
#if __FLT_MANT_DIG__ == 24
#define HAVE_FLOAT32
typedef float float32_t;
#define float32_suffix f
#define float_32(x)    (x##f)
#ifdef __HAVE_FAST_FMAF
#define __HAVE_FAST_FMA_32
#endif
#define fma_32(a, b, c) fmaf(a, b, c)
#define floor_32(x)     floorf(x)
#define frexp_32(x, a)  frexpf(x, a)
#define ldexp_32(x, a)  ldexpf(x, a)
#endif

#if __DBL_MANT_DIG__ == 53
#define HAVE_FLOAT64
typedef double float64_t;
#define float64_suffix
#define float_64(x) (x)
#ifdef __HAVE_FAST_FMA
#define __HAVE_FAST_FMA_64
#endif
#define fma_64(a, b, c) fma(a, b, c)
#define floor_64(x)     floor(x)
#define frexp_64(x, a)  frexp(x, a)
#define ldexp_64(x, a)  ldexp(x, a)
#elif __LDBL_MANT_DIG__ == 53
#define HAVE_FLOAT64
typedef long double float64_t;
#define float64_suffix l
#define float_64(x)    (x##l)
#ifdef __HAVE_FAST_FMAL
#define __HAVE_FAST_FMA_64
#endif
#define fma_64(a, b, c) fmal(a, b, c)
#define floor_64(x)     floorl(x)
#define frexp_64(x, a)  frexpl(x, a)
#define ldexp_64(x, a)  ldexpl(x, a)
#endif

#if __LDBL_MANT_DIG__ == 64
#define HAVE_FLOAT80
typedef long double float80_t;
#define float80_suffix l
#define float_80(x)    (x##l)
#ifdef __HAVE_FAST_FMAL
#define __HAVE_FAST_FMA_80
#endif
#define fma_80(a, b, c) fmal(a, b, c)
#define floor_80(x)     floorl(x)
#define frexp_80(x, a)  frexpl(x, a)
#define ldexp_80(x, a)  ldexpl(x, a)
#endif

#if __LDBL_MANT_DIG__ == 113
#define HAVE_FLOAT128
typedef long double float128_t;
#define float128_suffix l
#define float_128(x)    (x##l)
#ifdef __HAVE_FAST_FMAL
#define __HAVE_FAST_FMA_128
#endif
#define fma_128(a, b, c) fmal(a, b, c)
#define floor_128(x)     floorl(x)
#define frexp_128(x, a)  frexpl(x, a)
#define ldexp_128(x, a)  ldexpl(x, a)
#endif

#if defined(HAVE_FLOAT32) && defined(WANT_FLOAT32)

#define float_t           float32_t
#define fint_t            int32_t
#define fuint_t           uint32_t
#define NAME_SUFFIX       float32_suffix
#define KERNEL_SUFFIX     f
#define SPLIT_VAL         0x1p12
#define float_f(x)        float_32(x)
#define _isint_float_f(x) _isint_float_32(x)
#ifdef __HAVE_FAST_FMA_32
#define __HAVE_FAST_FMA_F
#endif

#include "math_two_inc.h"

#endif

#if defined(HAVE_FLOAT64) && defined(WANT_FLOAT64)

#define float_t     float64_t
#define fint_t      int64_t
#define fuint_t     uint64_t
#define NAME_SUFFIX float64_suffix
#define KERNEL_SUFFIX
#define SPLIT_VAL         0x1p26
#define float_f(x)        float_64(x)
#define _isint_float_f(x) _isint_float_64(x)
#ifdef __HAVE_FAST_FMA_64
#define __HAVE_FAST_FMA_F
#endif

#include "math_two_inc.h"

#endif

#if defined(HAVE_FLOAT80) && defined(WANT_FLOAT80)

#define float_t           float80_t
#define NAME_SUFFIX       float80_suffix
#define KERNEL_SUFFIX     l
#define SPLIT_VAL         0x1p32
#define float_f(x)        float_80(x)
#define _isint_float_f(x) _isint_float_80(x)
#ifdef __HAVE_FAST_FMA_80
#define __HAVE_FAST_FMA_F
#endif

#include "math_two_inc.h"

#endif

#if defined(HAVE_FLOAT128) && defined(WANT_FLOAT128)

#define float_t           float128_t
#define NAME_SUFFIX       float128_suffix
#define KERNEL_SUFFIX     l
#define SPLIT_VAL         0x1p57
#define float_f(x)        float_128(x)
#define _isint_float_f(x) _isint_float_128(x)
#ifdef __HAVE_FAST_FMA_128
#define __HAVE_FAST_FMA_F
#endif

#include "math_two_inc.h"

#endif

/*
 * inline functions to characterize floats which have no fractional
 * component.
 * isint = 0	... x is not an integer
 * isint = 1	... x is an odd int
 * isint = 2	... x is an even int
 */

#ifdef HAVE_FLOAT32

static inline int
_isint_float_32(float32_t x)
{
    __int32_t hx, ix, j, k;
    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7fffffff;
    if (ix >= 0x4b800000)
        return 2; /* even integer */
    else if (ix >= 0x3f800000) {
        k = (ix >> 23) - 0x7f; /* exponent */
        j = ix >> (23 - k);
        if (lsl(j, (23 - k)) == ix)
            return 2 - (j & 1);
    }
    return 0;
}

#endif /* HAVE_FLOAT32 */

#ifdef HAVE_FLOAT64

static inline int
_isint_float_64(float64_t x)
{
    __int32_t  hx, ix, j, k;
    __uint32_t lx;

    EXTRACT_WORDS(hx, lx, x);
    ix = hx & 0x7fffffff;

    if (ix >= 0x43400000)
        return 2; /* even integer y */
    else if (ix >= 0x3ff00000) {
        k = (ix >> 20) - 0x3ff; /* exponent */
        if (k > 20) {
            __uint32_t uj = lx >> (52 - k);
            if ((uj << (52 - k)) == lx)
                return 2 - (uj & 1);
        } else if (lx == 0) {
            j = ix >> (20 - k);
            if ((j << (20 - k)) == ix)
                return 2 - (j & 1);
        }
    }
    return 0;
}

#endif /* HAVE_FLOAT64 */

#ifdef HAVE_FLOAT80

static inline int
_isint_float_80(float80_t x)
{
    if (x != floor_80(x))
        return 0;
    x = ldexp_80(x, -1);
    if (x == floor_80(x))
        return 2;
    return 1;
}

#endif

#ifdef HAVE_FLOAT128

static inline int
_isint_float_128(float128_t x)
{
    if (x != floor_128(x))
        return 0;
    x = ldexp_128(x, -1);
    if (x == floor_128(x))
        return 2;
    return 1;
}

#endif

#endif /* _MATH_TWO_H_ */
