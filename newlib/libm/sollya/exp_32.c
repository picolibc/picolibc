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

#define WANT_FLOAT32

#include "math2.h"

/* 11 coefficients, error limits [2.7397320624086435310384596295057920011714910414849e-16;2.7397320624110574349649027684047439373846194484478e-16] */
static const ff_t exp_val[] = {
    { .hi = float_32(0x1p0), .lo = float_32(0x0p-2) },
    { .hi = float_32(0x1p0), .lo = float_32(0x1p-47) },
    { .hi = float_32(0x1p-1), .lo = float_32(-0x1p-46) },
    { .hi = float_32(0x1.555556p-3), .lo = float_32(-0x1.55697p-28) },
    { .hi = float_32(0x1.555556p-5), .lo = float_32(-0x1.5507a4p-30) },
    { .hi = float_32(0x1.111112p-7), .lo = float_32(-0x1.a3f1d8p-32) },
    { .hi = float_32(0x1.6c16cp-10), .lo = float_32(0x1.f5291p-35) },
    { .hi = float_32(0x1.a01978p-13), .lo = float_32(0x1.e0db68p-39) },
    { .hi = float_32(0x1.a01afp-16), .lo = float_32(-0x1.84ef58p-42) },
    { .hi = float_32(0x1.72fb18p-19), .lo = float_32(-0x1.bd99e8p-45) },
    { .hi = float_32(0x1.278944p-22), .lo = float_32(0x1.e2698p-49) },
};

static const float ln2_inv = 0x1.715476p0;
static const float halves[2] = { 0.5, -0.5 };
static const ff_t ln2 = {
    .hi = float_32(0x1.62e43p-1),
    .lo = float_32(-0x1.05c61p-29)
};

/* These are low by one */
static const fuint_t exp_plus[] = {
    0x33800000,
    0x383a3ef1,
    0x39c6be5b,
    0x39e5bb1d,
    0x40315b33,
    0x4034d02b,
    0x4178966e,
    0x41902323,
    0x41cbf87b,
    0x42339824,
    0x4283070f,
    0xbbf0edf1,
    0xbf81eadf,
    0xc16912cd,
    0xc29749e5,
    0xc2b27dd9,
    0xc2b2e798,
};

/* These are high by one */
static const fuint_t exp_minus[] = {
    0x3c608a0e,
    0x42441c1a,
    0x428a94c5,
    0x429675e7,
    0xbae0e25c,
    0xc21189a5,
    0xc233e0f0,
};

/* Binary search for a value */
static int
check_table(fuint_t value, const fuint_t *table, size_t size)
{
    _ssize_t     lo = 0, hi = size - 1;
    _ssize_t     mid;

    while (lo <= hi) {
        mid = (lo + hi) >> 1;
        if (table[mid] == value)
            return 1;
        if (table[mid] > value)
            hi = mid - 1;
        else
            lo = mid + 1;
    }
    return 0;
}

/* Mutate a float by one ULP */
static inline float
modify(float y, int off)
{
    return name(tofloat)((fuint_t) ((fint_t) name(touint)(y) + off));
}

/* Adjust the return value for a few special values which round incorrectly */
static inline float
table_adjust(float y, fint_t ix)
{
    if (check_table((fuint_t) ix, exp_plus, sizeof(exp_plus)/sizeof(exp_plus[0])))
        return modify(y, 1);
    if (check_table((fuint_t) ix, exp_minus, sizeof(exp_minus)/sizeof(exp_minus[0])))
        return modify(y, -1);
    return y;
}

float
expf(float x)
{
    fuint_t     ux = name(touint)(x);
    fint_t      ix = (fint_t) ux;
    fuint_t     sign = ux >> 31;

    ux &= 0x7fffffff;
    if (ux >= 0x7f800000) {
        if (ux > 0x7f800000)
            return x + x;
        if (sign)
            return 0;
        return x;
    }

    if (sign && ix > -0x3d300e4b)
        return __math_uflowf(0);

    if (ix > 0x42b17217)
        return __math_oflowf(0);

    int         expo = 0;
    ff_t        y = { .hi = x, .lo = 0 };

    /* reduce to range 0x1p-23 .. ln(2)/2 */
    if (ux > 0x3eb17218) {
        /* Compute exponent of result: round(x/log(2)) */
        expo = ln2_inv * x + halves[sign];
        y = dd_add_f(two_mul_ff(ln2, -expo), x);
    } else if (ux <= 0x33000000) {
        return __math_inexactf(1.0f + x);
    }

    /* Evaluate the polynomial */
    int         i = sizeof(exp_val) / sizeof (exp_val[0]) - 1;
    const ff_t  *c = &exp_val[i];
    ff_t        r = *c--;

    while (--i >= 0)
        r = dd_add_dd(two_mul_ff_ff(y, r), *c--);

    /* Apply the exponent and fine-tune the result */
    return table_adjust(name(_ff_scalbn)(r, expo), ix);
}
