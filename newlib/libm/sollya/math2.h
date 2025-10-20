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

#ifndef _MATH2_H_
#define _MATH2_H_

#include "fdlibm.h"
#include <math.h>
#include <inttypes.h>

/* Figure out mapping between C types and floating point formats */
#if __FLT_MANT_DIG__ == 24
#define HAVE_FLOAT32
typedef float float32_t;
#define float32_suffix  f
#define float_32(x)      (x ## f)
#ifdef FP_FAST_FMAF
#define HAVE_FAST_FMA_32
#endif
#define fma_32(a,b,c) fmaf(a,b,c)
#define floor_32(x)      floorf(x)
#define frexp_32(x,a)    frexpf(x,a)
#define ldexp_32(x,a)    ldexpf(x,a)
#endif

#if __DBL_MANT_DIG__ == 53
#define HAVE_FLOAT64
typedef double float64_t;
#define float64_suffix
#define float_64(x)     (x)
#ifdef HAVE_FAST_FMA
#define HAVE_FAST_FMA_64
#endif
#define fma_64(a,b,c) fma(a,b,c)
#define floor_64(x)      floor(x)
#define frexp_64(x,a)    frexp(x,a)
#define ldexp_64(x,a)    ldexp(x,a)
#elif __LDBL_MANT_DIG__ == 53
#define HAVE_FLOAT64
typedef long double float64_t;
#define float_64_suffix l
#define float_64(x)     (x ## l)
#ifdef HAVE_FAST_FMAL
#define HAVE_FAST_FMA_64
#endif
#define fma_64(a,b,c)   fmal(a,b,c)
#define floor_64(x)     floorl(x)
#define frexp_64(x,a)   frexpl(x,a)
#define ldexp_64(x,a)   ldexpl(x,a)
#endif

#if __LDBL_MANT_DIG__ == 64
#define HAVE_FLOAT80
typedef long double float80_t;
#define float80_suffix l
#define float_80(x)     (x ## l)
#ifdef HAVE_FAST_FMAL
#define HAVE_FAST_FMA_80
#endif
#define fma_80(a,b,c)   fmal(a,b,c)
#define floor_80(x)     floorl(x)
#define frexp_80(x,a)   frexpl(x,a)
#define ldexp_80(x,a)   ldexpl(x,a)
#endif

#if __LDBL_MANT_DIG__ == 113
#define HAVE_FLOAT128
typedef long double float128_t;
#define float128_suffix l
#define float_128(x)    (x ## l)
#ifdef HAVE_FAST_FMAL
#define HAVE_FAST_FMA_128
#endif
#define fma_128(a,b,c)  fmal(a,b,c)
#define floor_128(x)    floorl(x)
#define frexp_128(x,a)  frexpl(x,a)
#define ldexp_128(x,a)  ldexpl(x,a)
#endif

#if defined(HAVE_FLOAT32) && defined(WANT_FLOAT32)

#define float_t         float32_t
#define fint_t          int32_t
#define fuint_t         uint32_t
#define NAME_SUFFIX     float32_suffix
#define SPLIT_VAL       0x1p12
#define float_f(x)      float_32(x)
#ifdef HAVE_FAST_FMA_32
#define HAVE_FAST_FMA_F
#endif

#include "math2_inc.h"

#endif

#if defined(HAVE_FLOAT64) && defined(WANT_FLOAT64)

#define float_t         float64_t
#define fint_t          int64_t
#define fuint_t         uint64_t
#define NAME_SUFFIX     float64_suffix
#define SPLIT_VAL       0x1p26
#define float_f(x)      float_64(x)
#ifdef HAVE_FAST_FMA_64
#define HAVE_FAST_FMA_F
#endif

#include "math2_inc.h"

#endif

#if defined(HAVE_FLOAT80) && defined(WANT_FLOAT80)

#define float_t         float80_t
#define NAME_SUFFIX     float80_suffix
#define SPLIT_VAL       0x1p32
#define float_f(x)      float_80(x)
#ifdef HAVE_FAST_FMA_80
#define HAVE_FAST_FMA_F
#endif

#include "math2_inc.h"

#endif

#if defined(HAVE_FLOAT128) && defined(WANT_FLOAT128)

#define float_t         float128_t
#define NAME_SUFFIX     float128_suffix
#define SPLIT_VAL       0x1p57
#define float_f(x)      float_128(x)
#ifdef HAVE_FAST_FMA_128
#define HAVE_FAST_FMA_F
#endif

#include "math2_inc.h"

#endif

float __math_oflowf (uint32_t);
float __math_uflowf (uint32_t);
float __math_inexactf (float);

#endif /* _MATH2_H_ */
