/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
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

#include "math_ld.h"

#if !_HAVE_FAST_FMAL && (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113)

typedef long double FLOAT_T;

#define FMA fmal
#define NEXTAFTER nextafterl
#define LDEXP ldexpl
#define FREXP frexpl
#define SCALBN scalbnl
#define ILOGB    ilogbl
#define COPYSIGN copysignl

#if LDBL_MANT_DIG == 64
#define SPLIT (0x1p32L + 1.0L)
#if __LDBL_MIN_EXP__ == -16382
#define FLOAT_DENORM_BIAS       0
#endif
#endif
#if LDBL_MANT_DIG == 113
#define SPLIT (0x1p57L + 1.0L)
#endif

#define FLOAT_MANT_DIG        __LDBL_MANT_DIG__
#define FLOAT_MAX_EXP         __LDBL_MAX_EXP__
#define FLOAT_MIN             __LDBL_MIN__

static inline int
odd_mant(FLOAT_T x)
{
    union IEEEl2bits u;

    u.e = x;
    return u.bits.manl & 1;
}

static unsigned int
EXPONENT(FLOAT_T x)
{
    union IEEEl2bits u;

    u.e = x;
    return u.bits.exp;
}

#ifdef PICOLIBC_LONG_DOUBLE_NOEXCEPT
#define feraiseexcept(x) ((void) (x))
#endif

#include "fma_inc.h"

_MATH_ALIAS_d_ddd(fma)

#endif
