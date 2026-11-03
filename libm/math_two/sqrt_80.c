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
 * binary80 sqrtl using only integer arithmetic, computing the root
 * one bit at a time like sqrt and sqrtf in libm/math. The significand
 * and the partial root are held in the _u128 type from local-int128.h.
 */

#define _ISOC23_SOURCE
#define WANT_FLOAT80
#include "math_two.h"
#include "local-int128.h"

#ifdef float_t

#define SIG_ONE  ((uint64_t)1 << 63) /* explicit integer bit */
#define EXP_MASK 0x7fff
#define EXP_BIAS 0x3fff
#define SIG_BITS 64

float_t
name(sqrt)(float_t x)
{
    float_t  z;
    uint32_t se, i0, i1;
    uint64_t mx;
    _u128    ix, s, q, r, t;
    int      m;

    GET_LDOUBLE_WORDS(se, i0, i1, x);
    mx = ((uint64_t)i0 << 32) | i1;

    /* take care of Inf and NaN */
    if ((se & EXP_MASK) == EXP_MASK) {
        if ((se & 0x8000) && (mx << 1) == 0)
            return __math_invalidl(x); /* sqrt(-inf)=sNaN */
        return x + x;                  /* sqrt(NaN)=NaN, sqrt(+inf)=+inf */
    }

    /* take care of zero and -ves */
    if (mx == 0)
        return x; /* sqrt(+-0) = +-0 */
    if (se & 0x8000)
        return __math_invalidl(x); /* sqrt(-ve) = sNaN */

    /*
     * normalize x. The integer bit is explicit, so subnormal (and on
     * m68k, unnormal) values just need shifting until it is set.
     */
    m = se & EXP_MASK;
    if (m == 0)
        m = FLOAT_DENORM_BIAS;
    while ((mx & SIG_ONE) == 0) {
        mx <<= 1;
        m--;
    }
    m -= EXP_BIAS; /* unbias exponent */
    ix = to_u128(mx);
    if (m & 1) /* odd m, double x to make it even */
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

    /* q >> 1 has the integer bit at bit 63, unless rounding carried out */
    q = _u128_rshift(q, 1);
    if (!_u128_is_zero(_u128_rshift(q, SIG_BITS))) {
        q = _u128_rshift(q, 1);
        m++;
    }
    mx = from_u128(q);
    SET_LDOUBLE_WORDS(z, m + EXP_BIAS, (uint32_t)(mx >> 32), (uint32_t)mx);
    return z;
}

#endif
