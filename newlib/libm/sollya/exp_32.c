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

//#define USE_DOUBLE

#ifdef USE_DOUBLE

#if 0
static const double exp_val_d[] = {
         0x1p0 ,
         0x1p0 ,
         0x1p-1 ,
         0x1.555555555556p-3 ,
         0x1.55555555555ap-5 ,
         0x1.111111110c68p-7 ,
         0x1.6c16c16bd522p-10 ,
         0x1.a01a01d0e7e4p-13 ,
         0x1.a01a04518f9p-16 ,
         0x1.71dd3efad6ep-19 ,
         0x1.27d715a807eap-22 ,
         0x1.b3ba9b46ff8ap-26 ,
         0x1.6a8f8cca1578p-29 ,
         -0x1.aa058e3d2bb8p-30 ,
         -0x1.9f28dfba41cp-29 ,
         0x1.026d9e05a7fep-28 ,
         0x1.ca3c33ceff34p-28 ,
};
#endif

#if 0
static const double exp_val[] = {
    0x1p0 ,
    0x1.000000000002p0 ,
    0x1.fffffffffffp-2 ,
    0x1.555555554b48p-3 ,
    0x1.555555557c2ep-5 ,
    0x1.1111112e0714p-7 ,
    0x1.6c16c0fa9488p-10 ,
    0x1.a019787836dap-13 ,
    0x1.a01aef9ec42ap-16 ,
    0x1.72fb17909986p-19 ,
    0x1.2789443c4d3p-22 ,
};
#endif

/* too few to hit desired precision
static const double exp_val[] = {
         0x1.ffffffffff7ap-1 ,
         0x1.ffffffffd564p-1 ,
         0x1.00000000880ap-1 ,
         0x1.555555a0814cp-3 ,
         0x1.55555404e84ep-5 ,
         0x1.111082afa586p-7 ,
         0x1.6c18b37262f4p-10 ,
         0x1.a1a81b70fe8ap-13 ,
         0x1.9eda5fd11b0ap-16 ,
};

static const double exp_val[] = {
         0x1.000000000004p0 ,
         0x1.ffffffffffc6p-1 ,
         0x1.ffffffffe77p-2 ,
         0x1.55555555b7ap-3 ,
         0x1.555555884bcp-5 ,
         0x1.1111106226f2p-7 ,
         0x1.6c162c9bb452p-10 ,
         0x1.a01bb004f51ep-13 ,
         0x1.a17ce84b2e62p-16 ,
         0x1.70ece11b902ap-19 ,
};
*/

/* enough
static const double exp_val[] = {
    0x1p0 ,
    0x1.000000000002p0 ,
    0x1.fffffffffffp-2 ,
    0x1.555555554b48p-3 ,
    0x1.555555557c2ep-5 ,
    0x1.1111112e0714p-7 ,
    0x1.6c16c0fa9488p-10 ,
    0x1.a019787836dap-13 ,
    0x1.a01aef9ec42ap-16 ,
    0x1.72fb17909986p-19 ,
    0x1.2789443c4d3p-22 ,
};
*/

static const double exp_val_d[] = {
         0x1p0 ,
         0x1p0 ,
         0x1p-1 ,
         0x1.555555555556p-3 ,
         0x1.55555555555ap-5 ,
         0x1.111111110c68p-7 ,
         0x1.6c16c16bd522p-10 ,
         0x1.a01a01d0e7e4p-13 ,
         0x1.a01a04518f9p-16 ,
         0x1.71dd3efad6ep-19 ,
         0x1.27d715a807eap-22 ,
         0x1.b3ba9b46ff8ap-26 ,
         0x1.6a8f8cca1578p-29 ,
         -0x1.aa058e3d2bb8p-30 ,
         -0x1.9f28dfba41cp-29 ,
         0x1.026d9e05a7fep-28 ,
         0x1.ca3c33ceff34p-28 ,
};

#endif

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

static const fuint_t exp_minus[] = {
    0x3c608a0e,
    0x42441c1a,
    0x428a94c5,
    0x429675e7,
    0xbae0e25c,
    0xc21189a5,
    0xc233e0f0,
};


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

static inline float
modify(float y, int off)
{
    return name(tofloat)((fuint_t) ((fint_t) name(touint)(y) + off));
}

static inline float
table_adjust(float y, fint_t ix)
{
    if (check_table((fuint_t) ix, exp_plus, sizeof(exp_plus)/sizeof(exp_plus[0])))
        return modify(y, 1);
    if (check_table((fuint_t) ix, exp_minus, sizeof(exp_minus)/sizeof(exp_minus[0])))
        return modify(y, -1);
    return y;
}

#if 0
/* 17 coefficients, error limits [8.3010080268316599355386873133039311294590803977096e-20;8.3010080268389737295203471965902631564914507705466e-20] */
static const ff_t exp_val[] = {
        { .hi = float_32(0x1p0), .lo = float_32(0x0p-2) },
        { .hi = float_32(0x1p0), .lo = float_32(0x0p-2) },
        { .hi = float_32(0x1p-1), .lo = float_32(0x0p-2) },
        { .hi = float_32(0x1.555556p-3), .lo = float_32(-0x1.555554p-28) },
        { .hi = float_32(0x1.555556p-5), .lo = float_32(-0x1.55554cp-30) },
        { .hi = float_32(0x1.111112p-7), .lo = float_32(-0x1.dde73p-32) },
        { .hi = float_32(0x1.6c16c2p-10), .lo = float_32(-0x1.2855bcp-35) },
        { .hi = float_32(0x1.a01a02p-13), .lo = float_32(-0x1.78c0ep-40) },
        { .hi = float_32(0x1.a01a04p-16), .lo = float_32(0x1.463e4p-42) },
        { .hi = float_32(0x1.71dd3ep-19), .lo = float_32(0x1.f5adcp-44) },
        { .hi = float_32(0x1.27d716p-22), .lo = float_32(-0x1.5fe058p-48) },
        { .hi = float_32(0x1.b3ba9cp-26), .lo = float_32(-0x1.7200ecp-51) },
        { .hi = float_32(0x1.6a8f8cp-29), .lo = float_32(0x1.942afp-54) },
        { .hi = float_32(-0x1.aa058ep-30), .lo = float_32(-0x1.e95dcp-57) },
        { .hi = float_32(-0x1.9f28ep-29), .lo = float_32(0x1.16f9p-55) },
        { .hi = float_32(0x1.026d9ep-28), .lo = float_32(0x1.69ff8p-58) },
        { .hi = float_32(0x1.ca3c34p-28), .lo = float_32(-0x1.88066p-55) },
};
#endif

static const float ln2_inv = 0x1.715476p0;
static const float halves[2] = { 0.5, -0.5 };

#ifdef USE_DOUBLE
static const double d_ln2 = 0x1.62e42fefa39efp-1;
#endif
static const ff_t ln2 = { .hi = float_32(0x1.62e43p-1), .lo = float_32(-0x1.05c61p-29) };

#ifdef USE_DOUBLE
#include <stdarg.h>
static void
check_value(double want, ff_t got_ff, char *format, ...)
{
    double got = (double) got_ff.hi + (double) got_ff.lo;
    if (want == got)
        return;

    double err = fabs(want - got);

    int exp_want, exp_got, exp_err;
    frexp(want, &exp_want);
    frexp(got, &exp_got);
    frexp(err, &exp_err);

    if (exp_err > exp_want - 30) {
        va_list va;
        printf("want %a got %a exp_err %d exp_want %d\n", want, got, exp_err, exp_want);
        va_start(va, format);
        vprintf(format, va);
        va_end(va);
    }
}
#endif

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

    int32_t    expo = 0;
#ifdef USE_DOUBLE
    double      d_y = (double) x;
#endif
    ff_t        y = { .hi = x, .lo = 0 };

    /* reduce to range 0x1p-23 .. ln(2)/2 */
    if (ux > 0x3eb17218) {
        /* Compute exponent of result: round(x/log(2)) */
        expo = ln2_inv * x + halves[sign];
#ifdef USE_DOUBLE
        d_y = (double) x - d_ln2 * (double) expo;
#endif
        y = dd_add_f(two_mul_ff(ln2, -expo), x);
    } else if (ux <= 0x33000000) {
        return __math_inexactf(1.0f + x);
    }

    int         i = sizeof(exp_val) / sizeof (exp_val[0]) - 1;
#ifdef USE_DOUBLE
    const double *d_c = &exp_val_d[i];
    double      d_r = *d_c--;
#endif
    const ff_t  *c = &exp_val[i];
    ff_t        r = *c--;

    while (--i >= 0) {
        r = dd_add_dd(two_mul_ff_ff(y, r), *c--);
#ifdef USE_DOUBLE
        d_r = d_y * d_r + *d_c--;
//        if (i < 3)
//            check_value(d_r, r, "x %a loop %d\n", (double) x, i);
#endif
    }
    float       ret = (float) ldexp((double) r.hi + (double) r.lo, expo);
#ifdef USE_DOUBLE
    float       d_ret = (float) ldexp(d_r, expo);
//    check_value(d_r, r, "x %a loop end\n", (double) x);
    (void) check_value;
    if (ret != d_ret) {
        printf("                     ldexp differs got %a want %a r %a + %a d_r %a expo %d\n",
               (double) ret, (double) d_ret,
               (double) r.hi, (double) r.lo, d_r, expo);
    }
#endif
    return table_adjust(ret, ix);
}
