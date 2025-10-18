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

#define _GNU_SOURCE
#include "math2.h"

#ifdef float_t
static const float_t gamma_coeffs[] = COEFFS;

static const ff_t pi_ff = PI_FF;

#ifdef WANT_FLOAT32
static inline ff_t
split_d(double x)
{
    float       a = (float) ((double) SPLIT * x);
    float       b = (float) (x - (double) a);
    float       hi = a + b;
    float       lo = (float) (x - (double) hi);
    return (ff_t) { .hi = hi, .lo = lo };
}

static inline double
combine_f(ff_t f)
{
    return (double) f.hi + (double) f.lo;
}

#endif

#include <stdio.h>
#include <stdlib.h>
float_t
name(tgamma)(float_t x)
{
    /* non-finite values */
    if (!isfinite(x))
        return x + x;

#ifdef WANT_FLOAT32_
    float       one_f = 0x1.00001p0f;
    float       two_f = 0x2.00002p0f;

    printf("add %a + %a = %a (%a)\n",
           (double) one_f, (double) two_f,
           combine_f(two_add(one_f, two_f)),
           (double) one_f + (double) two_f);
    printf("mul %a * %a = %a (%a)\n",
           (double) one_f, (double) two_f,
           combine_f(two_mul(one_f, two_f)),
           (double) one_f * (double) two_f);

    double      one_d = 0x1.000000001p0;
    ff_t        one_e = split_d(one_d);

    printf("combine %a (%a+%a) to %a\n", one_d, (double) one_e.hi, (double) one_e.lo, combine_f(one_e));

    double      two_d = 0x2.000000002p0;
    ff_t        two_e = split_d(two_d);

    printf("combine %a (%a+%a) to %a\n", two_d, (double) two_e.hi, (double) two_e.lo, combine_f(two_e));

    printf("add %a + %a = %a (%a)\n",
           one_d, two_d, combine_f(dd_add_dd(one_e, two_e)), one_d + two_d);

    printf("mul %a * %a = %a (%a)\n",
           one_d, two_d, combine_f(two_mul_ff_ff(one_e, two_e)), one_d * two_d);

    double      three_d = two_d + one_d;
    ff_t        three_e = split_d(three_d);

    printf("div %a / %a = %a (%a)\n",
           two_d, three_d, combine_f(ff_div_ff(two_e, three_e)), two_d / three_d);

#if 0
    ff_t        three_e = dd_add_dd(one_e,two_e);
    double      ans = combine_f(three_e);
    printf("add %a + %a = %a (%a)\n", one_d, two_d, ans, one_d + two_d);
    printf("mul %a + %a = %a (%a)\n", one_d, two_d, combine_f(two_mul_ff_ff(one_e, two_e)), one_d * two_d);
#endif
    exit(0);
#endif

    /* non-positive integers return NAN */
    if (x <= 0 && floor_f(x) == x)
        return (float_t) NAN;

    /* Large positive values overflow */
    if (x > float_f(MAX_ARG))
        return (float_t) INFINITY;

    float_t     p = float_f(0.5);
    float_t     eps = 0;
    ff_t        ff, gg;

#define mix_in(v) do {                          \
        ff = two_mul(p, (v));                   \
        p = ff.hi;                              \
        eps += ff.lo / p;                       \
    } while(0)

    if (x < float_f(-0.5)) {
        float_t sp = name(sinpi)(x);
        float_t g = name(tgamma)(1-x);
        ff = two_mul(sp, g);
        gg = ff_div_ff(pi_ff, ff);
        return gg.hi;
    } else {
        while (x >= 2) {
            x -= 1;
            ff = two_mul(p, x);
            p = ff.hi;
            if (isinf(p))
                return p;
            eps += ff.lo / p;
        }
    }
    while (x < 1) {
        ff = two_mul(p, 1/x);
        p = ff.hi;
        if (isinf(p) || p == 0)
            return p;
        eps += ff.lo / p;
        x += 1;
    }

    int         i = sizeof(gamma_coeffs) / sizeof (gamma_coeffs[0]) - 1;
    float_t     r = gamma_coeffs[i];
    float_t     y = x - 1;

    while (--i >= 0)
        r = y * r + gamma_coeffs[i];

    r = r * p * (1 + eps) * 2;
    return r;
}
#endif
