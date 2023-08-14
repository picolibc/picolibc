/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
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

#define __STDC_WANT_IEC_60559_BFP_EXT__
#include <math.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fenv.h>

#if DBL_MANT_DIG == 53

static float
fmaf_slow(float x, float y, float z)
{
    return (float) ((double) x * (double) y + (double) z);
}

static const float valuef = 0x1.000002p0;

static bool
equalf(float x, float y)
{
    if (isnan(x) && isnan(y))
        return true;
    return x == y;
}

static int roundings[] = {
    -1,
#ifdef FE_TONEAREST
    FE_TONEAREST,
#endif
#ifdef FE_UPWARD
    FE_UPWARD,
#endif
#ifdef FE_DOWNWARD
    FE_DOWNWARD,
#endif
#ifdef FE_TOWARDZERO
    FE_TOWARDZERO,
#endif
};

static const char *rounding_names[] = {
    "default",
#ifdef FE_TONEAREST
    "FE_TONEAREST",
#endif
#ifdef FE_UPWARD
    "FE_UPWARD",
#endif
#ifdef FE_DOWNWARD
    "FE_DOWNWARD",
#endif
#ifdef FE_TOWARDZERO
    "FE_TOWARDZERO",
#endif
};

#define NROUND  (sizeof(roundings)/sizeof(roundings[0]))

#ifdef PICOLIBC_DOUBLE_NOEXCEPT
/*
 * Assume that a lack of exceptions for doubles also means a lack of
 * support for non-default rounding modes for doubles.
 */
#define SKIP_ROUNDINGS_FLOAT
#endif

#ifdef SKIP_ROUNDINGS_FLOAT
#define NROUND_FLOAT    1
#else
#define NROUND_FLOAT NROUND
#endif

#define FIRST_EXPF      (FLT_MIN_EXP - FLT_MANT_DIG - 2)
#define LAST_EXPF       (FLT_MAX_EXP + 2)

/*
 * We want to test transitional exponent values, so we test +/-2
 * relative relative to four points:
 *
 *  minimum
 *  smallest normal
 *  zero
 *  maximum
 */

static int
next_expf(int e)
{
    switch (e) {
    case FIRST_EXPF + 4:
        return FLT_MIN_EXP - 2;
    case FLT_MIN_EXP + 2:
        return -2;
    case 2:
        return LAST_EXPF - 4;
    default:
        return e + 1;
    }
}

#if defined(__PICOLIBC__) && !defined(TINY_STDIO)
static
int strfromf(char *str, size_t n, const char *format, float f)
{
    return snprintf(str, n, format, (double) f);
}
#endif

static int
test_fmaf(void)
{
    int x_exp, y_exp, z_exp;
    char        xs[20], ys[20], zs[20], rs[20], ss[20];
    int x_sign, y_sign, z_sign;
    int ret = 0;
    unsigned int ro;
    int defround = fegetround();

    for (ro = 0; ro < NROUND_FLOAT; ro++) {
        if (roundings[ro] == -1)
            fesetround(defround);
        else
            fesetround(roundings[ro]);
        printf("float %s\n", rounding_names[ro]);
        for (z_sign = -1; z_sign <= 1; z_sign += 2) {
            for (z_exp = FIRST_EXPF; z_exp <= LAST_EXPF; z_exp = next_expf(z_exp)) {
                float z = ldexpf(valuef * z_sign, z_exp);
                for (y_sign = -1; y_sign <= 1; y_sign += 2) {
                    for (y_exp = FIRST_EXPF; y_exp <= LAST_EXPF; y_exp = next_expf(y_exp)) {
                        float y = ldexpf(valuef * y_sign, y_exp);
                        for (x_sign = -1; x_sign <= 1; x_sign += 2) {
                            for (x_exp = FIRST_EXPF; x_exp <= LAST_EXPF; x_exp = next_expf(x_exp)) {
                                float x = ldexpf(valuef * x_sign, x_exp);

                                float r = fmaf(x, y, z);
                                float s = fmaf_slow(x, y, z);
                                if (!equalf(r, s)) {
                                    strfromf(xs, sizeof(xs), "%a", x);
                                    strfromf(ys, sizeof(xs), "%a", y);
                                    strfromf(zs, sizeof(xs), "%a", z);
                                    strfromf(rs, sizeof(xs), "%a", r);
                                    strfromf(ss, sizeof(xs), "%a", s);
                                    printf("round %s slow %s %s %s -> got %s want %s\n",
                                           rounding_names[ro],
                                           xs, ys, zs, rs, ss);
                                    ret = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    fesetround(defround);
    return ret;
}

#else
#define test_fmaf() 0
#endif

/*
 * If long double is large enough, we can use it to test
 * double FMA
 */
#if LDBL_MANT_DIG  == 113 && LDBL_MAX_EXP > DBL_MAX_EXP

static double
fma_slow(double x, double y, double z)
{
    return (double) ((long double) x * (long double) y + (long double) z);
}

#define FIRST_EXP      (DBL_MIN_EXP - DBL_MANT_DIG - 2)
#define LAST_EXP       (DBL_MAX_EXP + 1)

#define RANGE_EXP      10

/*
 * We want to test transitional exponent values, so we test +/-2
 * relative relative to four points:
 *
 *  minimum
 *  smallest normal
 *  zero
 *  maximum
 */

static int
next_exp(int e)
{
    switch (e) {
    case FIRST_EXP + 4:
        return DBL_MIN_EXP - 2;
    case DBL_MIN_EXP + 2:
        return -2;
    case 2:
        return LAST_EXP - 4;
    default:
        return e + 1;
    }
}

static bool
equal(double x, double y)
{
    if (isnan(x) && isnan(y))
        return true;
    return x == y;
}

static const double value = 0x1.0000000000001p0;

#ifdef PICOLIBC_LONG_DOUBLE_NOEXCEPT
/*
 * Assume that a lack of exceptions for doubles also means a lack of
 * support for non-default rounding modes for doubles.
 */
#define SKIP_ROUNDINGS_DOUBLE
#endif

#ifdef SKIP_ROUNDINGS_DOUBLE
#define NROUND_DOUBLE    1
#else
#define NROUND_DOUBLE NROUND
#endif

static int
test_fma(void)
{
    int x_exp, y_exp, z_exp;
    int x_sign, y_sign, z_sign;
    int ret = 0;
    unsigned int ro;
    int defround = fegetround();

    for (ro = 0; ro < NROUND_DOUBLE; ro++) {
        if (roundings[ro] == -1)
            fesetround(defround);
        else
            fesetround(roundings[ro]);
        printf("double %s\n", rounding_names[ro]);
        for (z_sign = -1; z_sign <= 1; z_sign += 2) {
            for (z_exp = FIRST_EXP; z_exp <= LAST_EXP; z_exp = next_exp(z_exp)) {
                double z = ldexp(value * z_sign, z_exp);
                for (y_sign = -1; y_sign <= 1; y_sign += 2) {
                    for (y_exp = FIRST_EXP; y_exp <= LAST_EXP; y_exp = next_exp(y_exp)) {
                        double y = ldexp(value * y_sign, y_exp);
                        for (x_sign = -1; x_sign <= 1; x_sign += 2) {
                            for (x_exp = FIRST_EXP; x_exp <= LAST_EXP; x_exp = next_exp(x_exp)) {
                                double x = ldexp(value * x_sign, x_exp);

                                double r = fma(x, y, z);
                                double s = fma_slow(x, y, z);
                                if (!equal(r, s)) {
                                    printf("round %s slow %a %a %a -> got %a want %a\n",
                                           rounding_names[ro], x, y, z, r, s);
                                    ret = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    fesetround(defround);
    return ret;
}

#else
#define test_fma() 0
#endif

int
main(void)
{
#ifdef __arc__
    volatile float x = 0x1.000002p-2f;
    volatile float y = 0x1.000002p-126f;
    volatile float z = 0x0.400002p-126f;
    if (x * y != z) {
        printf("ARC soft float bug, skipping FMA tests\n");
        return 77;
    }
#endif
    return test_fmaf() || test_fma();
}
