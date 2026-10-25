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

#ifdef __sh__
#if !(defined(__SH4__) || defined(__SH4_SINGLE__) || defined(__SH4_SINGLE_ONLY__))
#define GDB_SIMULATOR
#endif
#endif

#if FLT_MANT_DIG == 24

static int roundings[] = {
#ifdef FE_TONEAREST
    FE_TONEAREST,
#else
    -1,
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

static int rounding_map[] = {
#ifdef FE_TONEAREST
    0,
#endif
#ifdef FE_UPWARD
    1,
#endif
#ifdef FE_DOWNWARD
    2,
#endif
#ifdef FE_TOWARDZERO
    3,
#endif
};

static const char * const rounding_names[] = {
#ifdef FE_TONEAREST
    "FE_TONEAREST",
#else
    "default",
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

#define NROUND (sizeof(roundings) / sizeof(roundings[0]))

#ifdef __FLOAT_NOROUND
#define NROUND_FLOAT 1
#else
#define NROUND_FLOAT NROUND
#endif

#define nanf __builtin_nanf("")
#define inff INFINITY
#define nan  __builtin_nan("")
#define inf  ((double)INFINITY)
#define nanl __builtin_nanl("")
#define infl ((long double)INFINITY)

typedef struct {
    float r;
    bool  exact;
} float_ret;

struct fmaf_vec {
    unsigned  line;
    float     x, y, z;
    float_ret r[4];
};

typedef struct {
    double r;
    bool   exact;
} double_ret;

struct fma_vec {
    unsigned   line;
    double     x, y, z;
    double_ret r[4];
};

#if LDBL_MANT_DIG != 0
typedef struct {
    long double r;
    bool        exact;
} long_double_ret;

struct fmal_vec {
    unsigned        line;
    long double     x, y, z;
    long_double_ret r[4];
};
#endif

#ifdef _RX_PID
#define TEST_CONST
#else
#define TEST_CONST static const
#endif

#include "test-fma-vec.h"

#define NUM_FMAF_VEC (sizeof(fmaf_vec) / sizeof(fmaf_vec[0]))
#define NUM_FMA_VEC  (sizeof(fma_vec) / sizeof(fma_vec[0]))
#define NUM_FMAL_VEC (sizeof(fmal_vec) / sizeof(fmal_vec[0]))

#endif

#ifdef HAVE_FLOAT_FMA_VEC

#if defined(FE_INEXACT) && !defined(__FLOAT_NOEXCEPT)
#define FLOAT_CLEAR_INEXACT feclearexcept(FE_ALL_EXCEPT)
#define FLOAT_TEST_INEXACT  (fetestexcept(FE_INEXACT) == 0)
#else
#define FLOAT_CLEAR_INEXACT
#define FLOAT_TEST_INEXACT s_exact
#endif

static bool
equalf(float x, float y)
{
    if (isnan(x) && isnan(y))
        return true;
    return x == y;
}

static int
test_fmaf(void)
{
    char         xs[20], ys[20], zs[20], rs[20], ss[20];
    int          ret = 0;
    unsigned int ro;
    int          defround = fegetround();
    unsigned int t;

    printf("float\n");
    for (ro = 0; ro < NROUND_FLOAT; ro++) {
        if (roundings[ro] == -1)
            fesetround(defround);
        else
            fesetround(roundings[ro]);
        for (t = 0; t < NUM_FMAF_VEC; t++) {
            float x = fmaf_vec[t].x;
            float y = fmaf_vec[t].y;
            float z = fmaf_vec[t].z;
            float s = fmaf_vec[t].r[rounding_map[ro]].r;
            bool  s_exact = fmaf_vec[t].r[rounding_map[ro]].exact;
            bool  r_exact;
            FLOAT_CLEAR_INEXACT;
            volatile float r = fmaf(x, y, z);
            r_exact = FLOAT_TEST_INEXACT;
            if (!equalf(r, s) || r_exact != s_exact) {
                strfromf(xs, sizeof(xs), "%a", x);
                strfromf(ys, sizeof(xs), "%a", y);
                strfromf(zs, sizeof(xs), "%a", z);
                strfromf(rs, sizeof(xs), "%a", r);
                strfromf(ss, sizeof(xs), "%a", s);
                printf("%u: round %s %s * %s + %s -> got %s %s want %s %s\n", fmaf_vec[t].line,
                       rounding_names[ro], xs, ys, zs, rs, r_exact ? "exact" : "inexact", ss,
                       s_exact ? "exact" : "inexact");
                ret = 1;
            }
        }
    }
    fesetround(defround);
    return ret;
}
#else
#define test_fmaf() 0
#endif

#ifdef HAVE_DOUBLE_FMA_VEC

#if defined(FE_INEXACT) && !defined(__DOUBLE_NOEXCEPT)
#define DOUBLE_CLEAR_INEXACT feclearexcept(FE_ALL_EXCEPT)
#define DOUBLE_TEST_INEXACT  (fetestexcept(FE_INEXACT) == 0)
#else
#define DOUBLE_CLEAR_INEXACT
#define DOUBLE_TEST_INEXACT s_exact
#endif

#ifdef __DOUBLE_NOROUND
#define NROUND_DOUBLE 1
#else
#define NROUND_DOUBLE NROUND
#endif

static bool
equal(double x, double y)
{
    if (isnan(x) && isnan(y))
        return true;
    return x == y;
}

static int
test_fma(void)
{
    int          ret = 0;
    unsigned int ro;
    int          defround = fegetround();
    unsigned int t;

    printf("double\n");
    for (ro = 0; ro < NROUND_DOUBLE; ro++) {
        if (roundings[ro] == -1)
            fesetround(defround);
        else
            fesetround(roundings[ro]);
        for (t = 0; t < NUM_FMA_VEC; t++) {
            double x = fma_vec[t].x;
            double y = fma_vec[t].y;
            double z = fma_vec[t].z;
            double s = fma_vec[t].r[rounding_map[ro]].r;
            bool   s_exact = fma_vec[t].r[rounding_map[ro]].exact;
            bool   r_exact;
            DOUBLE_CLEAR_INEXACT;
            volatile double r = fma(x, y, z);
            r_exact = DOUBLE_TEST_INEXACT;
            if (!equal(r, s) || r_exact != s_exact) {
                printf("%u: round %s %a * %a + %a -> got %a %s want %a %s\n", fma_vec[t].line,
                       rounding_names[ro], x, y, z, r, r_exact ? "exact" : "inexact", s,
                       s_exact ? "exact" : "inexact");
                ret = 1;
            }
        }
    }
    fesetround(defround);
    return ret;
}
#else
#define test_fma() 0
#endif

#if defined(_TEST_LONG_DOUBLE) && defined(HAVE_LONG_DOUBLE_FMA_VEC)

#if defined(FE_INEXACT) && !defined(__LONG_DOUBLE_NOEXCEPT)
#define LONG_DOUBLE_CLEAR_INEXACT feclearexcept(FE_ALL_EXCEPT)
#define LONG_DOUBLE_TEST_INEXACT  (fetestexcept(FE_INEXACT) == 0)
#else
#define LONG_DOUBLE_CLEAR_INEXACT
#define LONG_DOUBLE_TEST_INEXACT s_exact
#endif

#ifdef __LONG_DOUBLE_NOROUND
#define NROUND_LONG_DOUBLE 1
#else
#define NROUND_LONG_DOUBLE NROUND
#endif

static bool
equall(long double x, long double y)
{
    if (isnan(x) && isnan(y))
        return true;
    return x == y;
}

static int
test_fmal(void)
{
    int          ret = 0;
    unsigned int ro;
    int          defround = fegetround();
    unsigned int t;

    printf("long double\n");
    for (ro = 0; ro < NROUND_LONG_DOUBLE; ro++) {
        if (roundings[ro] == -1)
            fesetround(defround);
        else
            fesetround(roundings[ro]);
        for (t = 0; t < NUM_FMAL_VEC; t++) {
            long double x = fmal_vec[t].x;
            long double y = fmal_vec[t].y;
            long double z = fmal_vec[t].z;
            long double s = fmal_vec[t].r[rounding_map[ro]].r;
            bool        s_exact = fmal_vec[t].r[rounding_map[ro]].exact;
            bool        r_exact;
            LONG_DOUBLE_CLEAR_INEXACT;
            volatile long double r = fmal(x, y, z);
            r_exact = LONG_DOUBLE_TEST_INEXACT;
            if (!equall(r, s) || r_exact != s_exact) {
                printf("%u: round %s %La * %La + %La -> got %La %s want %La %s\n", fmal_vec[t].line,
                       rounding_names[ro], x, y, z, r, r_exact ? "exact" : "inexact", s,
                       s_exact ? "exact" : "inexact");
                ret = 1;
            }
        }
    }
    fesetround(defround);
    return ret;
}
#else
#define test_fmal() 0
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
#ifdef GDB_SIMULATOR
    printf("GDB simulator doesn't support FMA. Skipping\n");
    return 77;
#endif
    (void)rounding_names;
    (void)roundings;
    int ret = 0;
    ret |= test_fmaf();
    ret |= test_fma();
#if defined(__m68k__) && __LDBL_MIN_EXP__ == -16382
    volatile long double big = 0x1p+16383l;
    volatile long double small = 0x1p-16446l;
    if (big * small != 0x1p-63) {
        printf("m68k qemu long double denorm bug, skipping long double tests\n");
        return ret;
    }
#endif
    ret |= test_fmal();
#ifdef __RX__
    if (ret) {
        printf("Expected failure on RX target, ignoring\n");
        ret = 77;
    }
#endif
    return ret;
}
