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
#include <stdbool.h>

#ifdef __GNUCLIKE_PRAGMA_DIAGNOSTIC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wignored-pragmas"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#pragma STDC FENV_ROUND FE_DYNAMIC
#pragma STDC FENV_ACCESS ON

/* Figure out mapping between C types and floating point formats */
#if __FLT_MANT_DIG__ == 24
#define HAVE_FLOAT32
typedef float float32_t;
#define float32_suffix      f
#define float_32(x)         (x##f)
#define FLOAT_32_MAX        FLT_MAX
#define FLOAT_32_MIN        FLT_MIN
#define FLOAT_32_DENORM_MIN __FLT_DENORM_MIN__
#define FLOAT_32_MAX_EXP    FLT_MAX_EXP
#ifdef __HAVE_FAST_FMAF
#define __HAVE_FAST_FMA_32
#endif
#ifdef __FLOAT_NOROUND
#define __FLOAT_32_NOROUND
#endif
#ifdef __FLOAT_NOEXCEPT
#define __FLOAT_32_NOEXCEPT
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
#define float_64(x)         (x)
#define FLOAT_64_MAX        DBL_MAX
#define FLOAT_64_MIN        DBL_MIN
#define FLOAT_64_DENORM_MIN __DBL_DENORM_MIN__
#define FLOAT_64_SNAN_BIT   0x0008000000000000ULL
#define FLOAT_64_MAX_EXP    DBL_MAX_EXP
#ifdef __HAVE_FAST_FMA
#define __HAVE_FAST_FMA_64
#endif
#ifdef __DOUBLE_NOROUND
#define __FLOAT_64_NOROUND
#endif
#ifdef __DOUBLE_NOEXCEPT
#define __FLOAT_64_NOEXCEPT
#endif

#define fma_64(a, b, c) fma(a, b, c)
#define floor_64(x)     floor(x)
#define frexp_64(x, a)  frexp(x, a)
#define ldexp_64(x, a)  ldexp(x, a)
#elif __LDBL_MANT_DIG__ == 53
#define HAVE_FLOAT64
typedef long double float64_t;
#define float64_suffix      l
#define float_64(x)         (x##l)
#define FLOAT_64_MAX        LDBL_MAX
#define FLOAT_64_MIN        LDBL_MIN
#define FLOAT_64_DENORM_MIN __LDBL_DENORM_MIN__
#define FLOAT_64_MAX_EXP    LDBL_MAX_EXP
#ifdef __HAVE_FAST_FMAL
#define __HAVE_FAST_FMA_64
#endif
#ifdef __LONG_DOUBLE_NOROUND
#define __FLOAT_64_NOROUND
#endif
#ifdef __LONG_DOUBLE_NOEXCEPT
#define __FLOAT_64_NOEXCEPT
#endif

#define fma_64(a, b, c) fmal(a, b, c)
#define floor_64(x)     floorl(x)
#define frexp_64(x, a)  frexpl(x, a)
#define ldexp_64(x, a)  ldexpl(x, a)
#endif

#if __LDBL_MANT_DIG__ == 64
#define HAVE_FLOAT80
typedef long double float80_t;
#define float80_suffix      l
#define float_80(x)         (x##l)
#define FLOAT_80_MAX        LDBL_MAX
#define FLOAT_80_MIN        LDBL_MIN
#define FLOAT_80_DENORM_MIN __LDBL_DENORM_MIN__
#define FLOAT_80_MAX_EXP    LDBL_MAX_EXP
#ifdef __HAVE_FAST_FMAL
#define __HAVE_FAST_FMA_80
#endif
#ifdef __LONG_DOUBLE_NOROUND
#define __FLOAT_80_NOROUND
#endif
#ifdef __LONG_DOUBLE_NOEXCEPT
#define __FLOAT_80_NOEXCEPT
#endif

#define fma_80(a, b, c) fmal(a, b, c)
#define floor_80(x)     floorl(x)
#define frexp_80(x, a)  frexpl(x, a)
#define ldexp_80(x, a)  ldexpl(x, a)
#endif

#if __LDBL_MANT_DIG__ == 113
#define HAVE_FLOAT128
typedef long double float128_t;
#define float128_suffix      l
#define float_128(x)         (x##l)
#define FLOAT_128_MAX        LDBL_MAX
#define FLOAT_128_MIN        LDBL_MIN
#define FLOAT_128_DENORM_MIN __LDBL_DENORM_MIN__
#define FLOAT_128_SNAN_BIT   0x0000800000000000ULL
#define FLOAT_128_MAX_EXP    LDBL_MAX_EXP
#ifdef __HAVE_FAST_FMAL
#define __HAVE_FAST_FMA_128
#endif
#ifdef __LONG_DOUBLE_NOROUND
#define __FLOAT_128_NOROUND
#endif
#ifdef __LONG_DOUBLE_NOEXCEPT
#define __FLOAT_128_NOEXCEPT
#endif

#define fma_128(a, b, c) fmal(a, b, c)
#define floor_128(x)     floorl(x)
#define frexp_128(x, a)  frexpl(x, a)
#define ldexp_128(x, a)  ldexpl(x, a)
#endif

#if defined(HAVE_FLOAT32) && defined(WANT_FLOAT32)

#define FLOAT_LEN         32
#define float_t           float32_t
#define fint_t            int32_t
#define fuint_t           uint32_t
#define NAME_SUFFIX       float32_suffix
#define KERNEL_SUFFIX     f
#define SPLIT_VAL         0x1p12
#define float_f(x)        float_32(x)
#define _isint_float_f(x) _isint_float_32(x)
#define FLOAT_MAX         FLOAT_32_MAX
#define FLOAT_MIN         FLOAT_32_MIN
#define FLOAT_DENORM_MIN  FLOAT_32_DENORM_MIN
#define FLOAT_MAX_EXP     FLOAT_32_MAX_EXP
#define FLOAT_SNAN_BIT    0x00400000u
#define FLOAT_EXP_MASK    0x7f800000U
#define FLOAT_EXP_BIAS    0x7f
#define FLOAT_EXP_INF     0xff
#define FLOAT_PAYLOAD_DIG 23
#ifdef __HAVE_FAST_FMA_32
#define __HAVE_FAST_FMA_F
#endif
#ifdef __FLOAT_32_NOEXCEPT
#define __FLOAT_NOEXCEPT_F
#endif
#ifdef __FLOAT_32_NOROUND
#define __FLOAT_NOROUND_F
#endif

#define GET_ABS(_ix)   ((_ix) & 0x7fffffff)
#define IS_FINITE(_ix) FLT_UWORD_IS_FINITE(_ix)
#define IS_NAN(_ix)    FLT_UWORD_IS_NAN(_ix)

#define GET_EXP(_ix, _x)         \
    do {                         \
        GET_FLOAT_WORD(_ix, _x); \
        (_ix) &= 0x7fffffff;     \
    } while (0)

#define EXP_IS_NONFINITE(_ix) (!FLT_UWORD_IS_FINITE(_ix))

#define EXP_ONE_QUARTER       0x3e800000

#define MAKE_SIG_ODD(_x)       \
    do {                       \
        int32_t w;             \
        GET_FLOAT_WORD(w, _x); \
        w |= 1;                \
        SET_FLOAT_WORD(_x, w); \
    } while (0)

#define GET_LOW_UINT32(v, x) GET_FLOAT_WORD(v, x)

#include "math_two_inc.h"

#endif

#if defined(HAVE_FLOAT64) && defined(WANT_FLOAT64)

#define FLOAT_LEN   64
#define float_t     float64_t
#define fint_t      int64_t
#define fuint_t     uint64_t
#define NAME_SUFFIX float64_suffix
#define KERNEL_SUFFIX
#define SPLIT_VAL         0x1p27
#define float_f(x)        float_64(x)
#define _isint_float_f(x) _isint_float_64(x)
#define FLOAT_MAX         FLOAT_64_MAX
#define FLOAT_MIN         FLOAT_64_MIN
#define FLOAT_DENORM_MIN  FLOAT_64_DENORM_MIN
#define FLOAT_MAX_EXP     FLOAT_64_MAX_EXP
#define FLOAT_SNAN_BIT    0x0008000000000000LL
#define FLOAT_EXP_MASK    0x7ff0000000000000LL
#define FLOAT_EXP_BIAS    0x3ff
#define FLOAT_EXP_INF     0x7ff
#define FLOAT_PAYLOAD_DIG 52
#ifdef __HAVE_FAST_FMA_64
#define __HAVE_FAST_FMA_F
#endif
#ifdef __FLOAT_64_NOEXCEPT
#define __FLOAT_NOEXCEPT_F
#endif
#ifdef __FLOAT_64_NOROUND
#define __FLOAT_NOROUND_F
#endif

#define GET_ABS(_ix) ((_ix) & 0x7fffffffffffffffLL)
#ifdef _FLT_LARGEST_EXPONENT_IS_NORMAL
#define IS_FINITE(_ix) 1
#define IS_INF(_ix)    0
#define IS_NAN(_ix)    0
#else
#define IS_FINITE(_ix) ((_ix) < 0x7ff0000000000000LL)
#define IS_INF(_ix)    ((_ix) == 0x7ff0000000000000LL)
#define IS_NAN(_ix)    ((_ix) > 0x7ff0000000000000LL)
#endif

#define GET_EXP(_ix, _x)        \
    do {                        \
        GET_HIGH_WORD(_ix, _x); \
        (_ix) &= 0x7fffffff;    \
    } while (0)

#define EXP_IS_NONFINITE(_ix) ((_ix) >= 0x7ff00000)

#define EXP_ONE_QUARTER       0x3fd00000

#define MAKE_SIG_ODD(_x)       \
    do {                       \
        uint32_t low;          \
        GET_LOW_WORD(low, _x); \
        low |= 1;              \
        SET_LOW_WORD(_x, low); \
    } while (0)

#define GET_LOW_UINT32(v, x) GET_LOW_WORD(v, x)

#include "math_two_inc.h"

#endif

#if defined(HAVE_FLOAT80) && defined(WANT_FLOAT80)

#define FLOAT_LEN         80
#define float_t           float80_t
#define NAME_SUFFIX       float80_suffix
#define KERNEL_SUFFIX     l
#define SPLIT_VAL         0x1p32
#define float_f(x)        float_80(x)
#define _isint_float_f(x) _isint_float_80(x)
#define FLOAT_MAX         FLOAT_80_MAX
#define FLOAT_MIN         FLOAT_80_MIN
#define FLOAT_EXP_BIAS    0x3fff
#define FLOAT_EXP_INF     0x7fff
#define FLOAT_SNAN_BIT    0x4000000000000000ULL
#define FLOAT_DENORM_MIN  FLOAT_80_DENORM_MIN
#define FLOAT_MAX_EXP     FLOAT_80_MAX_EXP
#define FLOAT_PAYLOAD_DIG 64
#ifdef __HAVE_FAST_FMA_80
#define __HAVE_FAST_FMA_F
#endif
#ifdef __FLOAT_80_NOEXCEPT
#define __FLOAT_NOEXCEPT_F
#endif
#ifdef __FLOAT_80_NOROUND
#define __FLOAT_NOROUND_F
#endif

typedef ieee_extended_shape_type fuint_t;

#define GET_EXP(_ix, _x)                    \
    do {                                    \
        u_int32_t se, i0, i1;               \
                                            \
        GET_LDOUBLE_WORDS(se, i0, i1, x);   \
        (void)i1;                           \
        (_ix) = se & 0x7fff;                \
        (_ix) = ((_ix) << 16) | (i0 >> 16); \
    } while (0)

#define EXP_IS_NONFINITE(_ix) ((_ix) >= 0x7fff0000)

#define EXP_ONE_QUARTER       0x3fe80000

#define MAKE_SIG_ODD(_x)            \
    do {                            \
        uint32_t low;               \
        GET_LDOUBLE_LSW(low, (_x)); \
        low |= 1;                   \
        SET_LDOUBLE_LSW((_x), low); \
    } while (0)

#define GET_LOW_UINT32(v, x) GET_LDOUBLE_LSW(v, x)

#include "math_two_inc.h"

static inline fuint_t
f_touint(float_t x)
{
    fuint_t i;
    i.value = x;
    return i;
}

static inline float_t
f_tofloat(fuint_t i)
{
    return i.value;
}

static inline int
f_exp(fuint_t i)
{
    return (int)(i.parts.exp & 0x7fff) - FLOAT_EXP_BIAS;
}

static inline bool
f_is_minus(fuint_t i)
{
    return i.parts.exp < 0;
}

static inline bool
f_is_zero(fuint_t i)
{
    return (i.parts.exp & 0x7fff) == 0 && i.parts.sw == 0;
}

static inline bool
f_is_integer(fuint_t i, int shift)
{
    if (f_is_zero(i))
        return true;
    return (i.parts.sw & (((uint64_t)1 << (shift - 1)) - 1)) == 0;
}

#endif

#if defined(HAVE_FLOAT128) && defined(WANT_FLOAT128)

#define FLOAT_LEN         128
#define float_t           float128_t
#define NAME_SUFFIX       float128_suffix
#define KERNEL_SUFFIX     l
#define SPLIT_VAL         0x1p57
#define float_f(x)        float_128(x)
#define _isint_float_f(x) _isint_float_128(x)
#define FLOAT_MAX         FLOAT_128_MAX
#define FLOAT_MIN         FLOAT_128_MIN
#define FLOAT_EXP_BIAS    0x3fff
#define FLOAT_DENORM_MIN  FLOAT_128_DENORM_MIN
#define FLOAT_SNAN_BIT    0x0000800000000000ULL
#define FLOAT_EXP_INF     0x7fff
#define FLOAT_MAX_EXP     FLOAT_128_MAX_EXP
#define FLOAT_PAYLOAD_DIG 112
#ifdef __HAVE_FAST_FMA_128
#define __HAVE_FAST_FMA_F
#endif
#ifdef __FLOAT_128_NOEXCEPT
#define __FLOAT_NOEXCEPT_F
#endif
#ifdef __FLOAT_128_NOROUND
#define __FLOAT_NOROUND_F
#endif

#define FLOAT_ABS(_hx) ((_hx) & 0x7fffffffffffffffLL)
#ifdef _FLT_LARGEST_EXPONENT_IS_NORMAL
#define IS_FINITE(_hx, _lx) 1
#define IS_INF(_hx, _lx)    0
#define IS_NAN(_hx, _lx)    0
#else
#define IS_FINITE(_hx, _lx) ((_hx) < 0x7fff000000000000LL)
#define IS_INF(_hx, _lx)    ((_hx) == 0x7fff000000000000LL && (_lx) == 0)
#define IS_NAN(_hx, _lx)                                                            \
    ((_hx) > 0x7fff000000000000LL || ((_hx) == 0x7fff000000000000LL && (_lx) != 0))
#endif

#define GET_EXP(_ix, _x)          \
    do {                          \
        GET_LDOUBLE_EXP(_ix, _x); \
        (_ix) &= 0x7fff;          \
    } while (0)

#define EXP_IS_NONFINITE(_ix) ((_ix) >= 0x7fff)

#define EXP_ONE_QUARTER       0x3ff8

#define MAKE_SIG_ODD(_x)              \
    do {                              \
        uint64_t low;                 \
        GET_LDOUBLE_LSW64(low, (_x)); \
        low |= 1;                     \
        SET_LDOUBLE_LSW64((_x), low); \
    } while (0)

#define GET_LOW_UINT32(_v, _x)         \
    do {                               \
        uint64_t _low;                 \
        GET_LDOUBLE_LSW64(_low, (_x)); \
        (_v) = (uint32_t)_low;         \
    } while (0)

#include "math_two_inc.h"

typedef ieee_quad_shape_type fuint_t;

static inline fuint_t
f_touint(float_t x)
{
    fuint_t i;
    i.value = x;
    return i;
}

static inline float_t
f_tofloat(fuint_t i)
{
    return i.value;
}

static inline fuint_t
f_abs(fuint_t i)
{
    i.parts32.mswhi &= 0x7fffffffUL;
    return i;
}

static inline bool
f_is_minus(fuint_t i)
{
    return (int64_t)i.parts64.msw < 0;
}

static inline bool
f_is_zero(fuint_t i)
{
    return i.parts64.msw == 0 && i.parts64.lsw == 0;
}

static inline int
f_exp(fuint_t i)
{
    return (int)(i.parts64.msw >> (FLOAT_PAYLOAD_DIG - 64)) - FLOAT_EXP_BIAS;
}

static inline bool
f_is_nan(fuint_t i)
{
    i = f_abs(i);
    if (i.parts64.msw < 0x7fff000000000000ULL)
        return false;
    if (i.parts64.msw == 0x7fff000000000000ULL && i.parts64.lsw == 0)
        return false;
    return true;
}

static inline bool
f_is_integer(fuint_t pi, int shift)
{
    if (f_is_zero(pi))
        return true;
    if (shift >= 64) {
        if ((pi.parts64.msw & (((uint64_t)1 << (shift - 64)) - 1)) != 0)
            return false;
        if (pi.parts64.lsw != 0)
            return false;
        return true;
    }
    return (pi.parts64.lsw & (((uint64_t)1 << shift) - 1)) == 0;
}

static inline fuint_t
f_and(fuint_t i, fuint_t j)
{
    i.parts64.msw &= j.parts64.msw;
    i.parts64.lsw &= j.parts64.lsw;
    return i;
}

static inline fuint_t
f_or(fuint_t i, fuint_t j)
{
    i.parts64.msw |= j.parts64.msw;
    i.parts64.lsw |= j.parts64.lsw;
    return i;
}

static inline fuint_t
f_xor(fuint_t i, fuint_t j)
{
    i.parts64.msw ^= j.parts64.msw;
    i.parts64.lsw ^= j.parts64.lsw;
    return i;
}

static inline fuint_t
f_exp_or(fuint_t i, uint32_t exp)
{
    i.parts32.mswhi |= exp << 16;
    return i;
}

static inline fuint_t
f_exp_and(fuint_t i, uint32_t exp)
{
    i.parts32.mswhi &= exp << 16;
    return i;
}

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
