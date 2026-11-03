/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

/*
 * Based on fdlibm e_sqrt.c:
 *
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * binary128 sqrtl using only integer arithmetic, computing the root
 * one bit at a time like sqrt and sqrtf in libm/math. The significand
 * and the partial root are held in the _u128 type from local-int128.h.
 */

#define _ISOC23_SOURCE
#define WANT_FLOAT128
#include "math_two.h"
#include "local-int128.h"

#ifdef float_t

#define SIG_HIDDEN ((uint64_t)1 << 48) /* hidden bit in the high word */
#define SIG_MASK   (SIG_HIDDEN - 1)    /* explicit bits in the high word */
#define EXP_SHIFT  48
#define EXP_MASK   0x7fff
#define EXP_BIAS   0x3fff
#define SIG_BITS   113 /* significand bits, including hidden */

float_t
name(sqrt)(float_t x)
{
    float_t  z;
    uint64_t hx, lx;
    _u128    ix, s, q, r, t;
    int      m, i;

    GET_LDOUBLE_WORDS64(hx, lx, x);

    /* take care of Inf and NaN */
    if (((hx >> EXP_SHIFT) & EXP_MASK) == EXP_MASK) {
        if ((int64_t)hx < 0 && !isnan(x))
            return __math_invalidl(x); /* sqrt(-inf)=sNaN */
        return x + x;                  /* sqrt(NaN)=NaN, sqrt(+inf)=+inf */
    }

    /* take care of zero and -ves */
    if (((hx & ~((uint64_t)1 << 63)) | lx) == 0)
        return x; /* sqrt(+-0) = +-0 */
    if ((int64_t)hx < 0)
        return __math_invalidl(x); /* sqrt(-ve) = sNaN */

    /* normalize x */
    m = (int)(hx >> EXP_SHIFT);
    ix = _u128_or_64(_u128_lshift_64(hx & SIG_MASK, 64), lx);
    if (m == 0) { /* subnormal x */
        for (i = 0; (from_u128(_u128_rshift(ix, 64)) & SIG_HIDDEN) == 0; i++)
            ix = _u128_lshift(ix, 1);
        m -= i - 1;
    } else {
        ix = _u128_or(ix, _u128_lshift_64(SIG_HIDDEN, 64));
    }
    m -= EXP_BIAS; /* unbias exponent */
    if (m & 1)     /* odd m, double x to make it even */
        ix = _u128_plus(ix, ix);
    m >>= 1; /* m = [m/2] */

    /* generate sqrt(x) bit by bit */
    ix = _u128_plus(ix, ix);
    q = s = _u128_zero;               /* q = sqrt(x) */
    r = _u128_lshift_64(1, SIG_BITS); /* r = moving bit from right to left */

    while (!_u128_is_zero(r)) {
        t = _u128_plus(s, r);
        if (_u128_ge(ix, t)) {
            s = _u128_plus(t, r);
            ix = _u128_minus(ix, t);
            q = _u128_plus(q, r);
        }
        ix = _u128_plus(ix, ix);
        r = _u128_rshift(r, 1);
    }

    if (!_u128_is_zero(ix)) {
        __math_set_inexactl();
        FE_DECL_ROUND(rnd);
        if (__is_nearest(rnd))
            q = _u128_plus_64(q, _u128_and_64(q, 1));
        else if (__is_upward(rnd))
            q = _u128_plus_64(q, 2);
    }

    /*
     * q >> 1 has the hidden bit at bit 112. Adding the exponent for
     * 0.5 and the hidden bit together give the exponent for 1.0, and
     * a carry out of the rounding moves into the exponent too.
     */
    q = _u128_rshift(q, 1);
    hx = from_u128(_u128_rshift(q, 64)) + ((uint64_t)(EXP_BIAS - 1) << EXP_SHIFT);
    hx += (uint64_t)(int64_t)m << EXP_SHIFT;
    lx = from_u128(q);
    SET_LDOUBLE_WORDS64(z, hx, lx);
    return z;
}

#endif
