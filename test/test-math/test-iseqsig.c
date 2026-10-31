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

#define _ISOC23_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include <fenv.h>

/* The mixed-type checks rely on implicit promotion to the wider type */
#ifdef __GNUCLIKE_PRAGMA_DIAGNOSTIC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif

/*
 * iseqsig(x, y) returns x == y and raises FE_INVALID when either
 * operand is a NaN, quiet or signaling. Operands are converted to
 * their common real type, so this also checks the macro's dispatch on
 * mixed argument types.
 */

#ifdef FE_INVALID
#define CLEAR_INVALID() feclearexcept(FE_INVALID)
#define TEST_INVALID()  (fetestexcept(FE_INVALID) != 0)
#else
#define CLEAR_INVALID() ((void)0)
#define TEST_INVALID()  false
#endif

#if defined(FE_INVALID) && !defined(__FLOAT_NOEXCEPT)
#define CHECK_INVALID_FLOAT true
#else
#define CHECK_INVALID_FLOAT false
#endif

#if defined(FE_INVALID) && !defined(__DOUBLE_NOEXCEPT)
#define CHECK_INVALID_DOUBLE true
#else
#define CHECK_INVALID_DOUBLE false
#endif

#if defined(FE_INVALID) && !defined(__LONG_DOUBLE_NOEXCEPT)
#define CHECK_INVALID_LONG_DOUBLE true
#else
#define CHECK_INVALID_LONG_DOUBLE false
#endif

static int failures;

static void
check(const char *expr, int line, int got, int want, bool invalid, bool want_invalid,
      bool check_invalid)
{
    if (got != want) {
        printf("%d: %s returned %d, want %d\n", line, expr, got, want);
        failures++;
    }
    if (check_invalid && invalid != want_invalid) {
        printf("%d: %s %s FE_INVALID\n", line, expr, invalid ? "raised" : "did not raise");
        failures++;
    }
}

#define CHECK(kind, x, y, want, want_invalid)                                          \
    do {                                                                               \
        CLEAR_INVALID();                                                               \
        int  _got = iseqsig(x, y);                                                     \
        bool _invalid = TEST_INVALID();                                                \
        check("iseqsig(" #x ", " #y ")", __LINE__, _got, want, _invalid, want_invalid, \
              CHECK_INVALID_##kind);                                                   \
    } while (0)

static volatile float  f_zero = 0.0f, f_one = 1.0f, f_two = 2.0f, f_inf = INFINITY;
static volatile float  f_min = FLT_TRUE_MIN, f_max = FLT_MAX;
static volatile float  f_qnan = NAN, f_snan = __builtin_nansf("");
static volatile float  f_third = 1.0f / 3.0f;
static volatile double d_zero = 0.0, d_one = 1.0, d_two = 2.0, d_inf = INFINITY;
static volatile double d_min = DBL_TRUE_MIN, d_max = DBL_MAX;
static volatile double d_qnan = NAN, d_snan = __builtin_nans("");
static volatile double d_third = 1.0 / 3.0, d_f_third = (double)(1.0f / 3.0f);
static volatile double d_f_min = FLT_TRUE_MIN;
#if DBL_MANT_DIG > FLT_MANT_DIG
static volatile double d_f_min_half = (double)FLT_TRUE_MIN / 2;
static volatile double d_f_max_plus = (double)FLT_MAX * (1.0 + DBL_EPSILON);
#endif
static volatile double d_one_plus = 1.0 + DBL_EPSILON;

#ifdef _TEST_LONG_DOUBLE
static volatile long double l_zero = 0.0L, l_one = 1.0L, l_two = 2.0L, l_inf = INFINITY;
static volatile long double l_min = LDBL_TRUE_MIN, l_max = LDBL_MAX;
static volatile long double l_qnan = NAN, l_snan = __builtin_nansl("");
static volatile long double l_d_third = (long double)(1.0 / 3.0);
#if LDBL_MANT_DIG > DBL_MANT_DIG
static volatile long double l_third = 1.0L / 3.0L;
#endif
#if LDBL_MAX_EXP > DBL_MAX_EXP
static volatile long double l_d_max_plus = (long double)DBL_MAX * (1.0L + LDBL_EPSILON);
#endif
#if LDBL_MIN_EXP < DBL_MIN_EXP
static volatile long double l_d_min = DBL_TRUE_MIN;
static volatile long double l_d_min_half = (long double)DBL_TRUE_MIN / 2;
#endif
static volatile long double l_one_plus = 1.0L + LDBL_EPSILON;
#endif

static void
test_float(void)
{
    CHECK(FLOAT, f_one, f_one, 1, false);
    CHECK(FLOAT, f_one, f_two, 0, false);
    CHECK(FLOAT, f_two, f_one, 0, false);
    CHECK(FLOAT, f_zero, -f_zero, 1, false);
    CHECK(FLOAT, -f_zero, f_zero, 1, false);
    CHECK(FLOAT, f_min, f_zero, 0, false);
    CHECK(FLOAT, f_min, f_min, 1, false);
    CHECK(FLOAT, f_max, f_inf, 0, false);
    CHECK(FLOAT, f_inf, f_inf, 1, false);
    CHECK(FLOAT, -f_inf, f_inf, 0, false);
    CHECK(FLOAT, f_one, f_qnan, 0, true);
    CHECK(FLOAT, f_qnan, f_one, 0, true);
    CHECK(FLOAT, f_qnan, f_qnan, 0, true);
    CHECK(FLOAT, -f_qnan, f_inf, 0, true);
    CHECK(FLOAT, f_one, f_snan, 0, true);
    CHECK(FLOAT, f_snan, f_one, 0, true);
}

static void
test_double(void)
{
    CHECK(DOUBLE, d_one, d_one, 1, false);
    CHECK(DOUBLE, d_one, d_two, 0, false);
    CHECK(DOUBLE, d_two, d_one, 0, false);
    CHECK(DOUBLE, d_zero, -d_zero, 1, false);
    CHECK(DOUBLE, -d_zero, d_zero, 1, false);
    CHECK(DOUBLE, d_min, d_zero, 0, false);
    CHECK(DOUBLE, d_min, d_min, 1, false);
    CHECK(DOUBLE, d_max, d_inf, 0, false);
    CHECK(DOUBLE, d_inf, d_inf, 1, false);
    CHECK(DOUBLE, -d_inf, d_inf, 0, false);
    CHECK(DOUBLE, d_one, d_one_plus, 0, false);
    CHECK(DOUBLE, d_one, d_qnan, 0, true);
    CHECK(DOUBLE, d_qnan, d_one, 0, true);
    CHECK(DOUBLE, d_qnan, d_qnan, 0, true);
    CHECK(DOUBLE, -d_qnan, d_inf, 0, true);
    CHECK(DOUBLE, d_one, d_snan, 0, true);
    CHECK(DOUBLE, d_snan, d_one, 0, true);
}

static int evaluations;

static float
count_float(float x)
{
    evaluations++;
    return x;
}

static double
count_double(double x)
{
    evaluations++;
    return x;
}

/* Mixed float and double operands compare as double */
static void
test_float_double(void)
{
    CHECK(DOUBLE, f_one, d_one, 1, false);
    CHECK(DOUBLE, d_one, f_one, 1, false);
    CHECK(DOUBLE, f_two, d_one, 0, false);
    CHECK(DOUBLE, d_one, f_two, 0, false);
    CHECK(DOUBLE, f_zero, -d_zero, 1, false);
    CHECK(DOUBLE, -f_zero, d_zero, 1, false);
    CHECK(DOUBLE, f_inf, d_inf, 1, false);
    CHECK(DOUBLE, d_inf, f_inf, 1, false);
    CHECK(DOUBLE, -f_inf, d_inf, 0, false);
    CHECK(DOUBLE, f_third, d_f_third, 1, false);
    CHECK(DOUBLE, d_f_third, f_third, 1, false);
    CHECK(DOUBLE, f_min, d_f_min, 1, false);
#if DBL_MANT_DIG > FLT_MANT_DIG
    /*
     * Values which differ only when not rounded to float. Each
     * appears in both orders, so converting both operands to the
     * type of the first, the second or the narrower one fails.
     */
    CHECK(DOUBLE, f_one, d_one_plus, 0, false);
    CHECK(DOUBLE, d_one_plus, f_one, 0, false);
    CHECK(DOUBLE, f_third, d_third, 0, false);
    CHECK(DOUBLE, d_third, f_third, 0, false);
    CHECK(DOUBLE, f_max, d_f_max_plus, 0, false);
    CHECK(DOUBLE, d_f_max_plus, f_max, 0, false);
    CHECK(DOUBLE, f_max, d_max, 0, false);
    CHECK(DOUBLE, d_max, f_max, 0, false);
    CHECK(DOUBLE, f_inf, d_max, 0, false);
    CHECK(DOUBLE, d_max, f_inf, 0, false);
    CHECK(DOUBLE, f_zero, d_min, 0, false);
    CHECK(DOUBLE, d_min, f_zero, 0, false);
    CHECK(DOUBLE, f_min, d_f_min_half, 0, false);
    CHECK(DOUBLE, d_f_min_half, f_min, 0, false);
    CHECK(DOUBLE, f_zero, d_f_min_half, 0, false);
    CHECK(DOUBLE, d_f_min_half, f_zero, 0, false);
#endif
    CHECK(DOUBLE, f_one, d_qnan, 0, true);
    CHECK(DOUBLE, d_qnan, f_one, 0, true);
    CHECK(DOUBLE, d_one, f_qnan, 0, true);
    CHECK(DOUBLE, f_qnan, d_one, 0, true);
    CHECK(DOUBLE, f_qnan, d_qnan, 0, true);
    CHECK(DOUBLE, f_snan, d_one, 0, true);
    CHECK(DOUBLE, d_one, f_snan, 0, true);
    CHECK(DOUBLE, f_one, d_snan, 0, true);

    /* Each operand is evaluated exactly once */
    evaluations = 0;
    CHECK(DOUBLE, count_float(f_one), count_double(d_one), 1, false);
    CHECK(DOUBLE, count_double(d_two), count_float(f_one), 0, false);
    if (evaluations != 4) {
        printf("%d: iseqsig evaluated operands %d times, want 4\n", __LINE__, evaluations);
        failures++;
    }
}

#ifdef _TEST_LONG_DOUBLE
static void
test_long_double(void)
{
    CHECK(LONG_DOUBLE, l_one, l_one, 1, false);
    CHECK(LONG_DOUBLE, l_one, l_two, 0, false);
    CHECK(LONG_DOUBLE, l_two, l_one, 0, false);
    CHECK(LONG_DOUBLE, l_zero, -l_zero, 1, false);
    CHECK(LONG_DOUBLE, -l_zero, l_zero, 1, false);
    CHECK(LONG_DOUBLE, l_min, l_zero, 0, false);
    CHECK(LONG_DOUBLE, l_min, l_min, 1, false);
    CHECK(LONG_DOUBLE, l_max, l_inf, 0, false);
    CHECK(LONG_DOUBLE, l_inf, l_inf, 1, false);
    CHECK(LONG_DOUBLE, -l_inf, l_inf, 0, false);
    CHECK(LONG_DOUBLE, l_one, l_one_plus, 0, false);
    CHECK(LONG_DOUBLE, l_one, l_qnan, 0, true);
    CHECK(LONG_DOUBLE, l_qnan, l_one, 0, true);
    CHECK(LONG_DOUBLE, l_qnan, l_qnan, 0, true);
    CHECK(LONG_DOUBLE, -l_qnan, l_inf, 0, true);
    CHECK(LONG_DOUBLE, l_one, l_snan, 0, true);
    CHECK(LONG_DOUBLE, l_snan, l_one, 0, true);
}

/* Mixed operands with a long double compare as long double */
static void
test_mixed_long_double(void)
{
    CHECK(LONG_DOUBLE, f_one, l_one, 1, false);
    CHECK(LONG_DOUBLE, l_one, f_one, 1, false);
    CHECK(LONG_DOUBLE, d_one, l_one, 1, false);
    CHECK(LONG_DOUBLE, l_one, d_one, 1, false);
    CHECK(LONG_DOUBLE, f_two, l_one, 0, false);
    CHECK(LONG_DOUBLE, l_one, d_two, 0, false);
    CHECK(LONG_DOUBLE, f_zero, -l_zero, 1, false);
    CHECK(LONG_DOUBLE, -d_zero, l_zero, 1, false);
    CHECK(LONG_DOUBLE, f_inf, l_inf, 1, false);
    CHECK(LONG_DOUBLE, l_inf, d_inf, 1, false);
    CHECK(LONG_DOUBLE, -d_inf, l_inf, 0, false);
    CHECK(LONG_DOUBLE, d_third, l_d_third, 1, false);
    CHECK(LONG_DOUBLE, l_d_third, d_third, 1, false);
#if LDBL_MANT_DIG > DBL_MANT_DIG
    /*
     * Values which differ only when not rounded to double or float,
     * in both orders as above
     */
    CHECK(LONG_DOUBLE, d_one, l_one_plus, 0, false);
    CHECK(LONG_DOUBLE, l_one_plus, d_one, 0, false);
    CHECK(LONG_DOUBLE, f_one, l_one_plus, 0, false);
    CHECK(LONG_DOUBLE, l_one_plus, f_one, 0, false);
    CHECK(LONG_DOUBLE, d_third, l_third, 0, false);
    CHECK(LONG_DOUBLE, l_third, d_third, 0, false);
    CHECK(LONG_DOUBLE, f_third, l_third, 0, false);
    CHECK(LONG_DOUBLE, l_third, f_third, 0, false);
#endif
#if LDBL_MAX_EXP > DBL_MAX_EXP
    /* Values outside the range of double */
    CHECK(LONG_DOUBLE, d_max, l_d_max_plus, 0, false);
    CHECK(LONG_DOUBLE, l_d_max_plus, d_max, 0, false);
    CHECK(LONG_DOUBLE, d_inf, l_max, 0, false);
    CHECK(LONG_DOUBLE, l_max, d_inf, 0, false);
    CHECK(LONG_DOUBLE, f_inf, l_max, 0, false);
    CHECK(LONG_DOUBLE, l_max, f_inf, 0, false);
#endif
#if LDBL_MIN_EXP < DBL_MIN_EXP
    /* Values which underflow to zero in double or float, in both orders */
    CHECK(LONG_DOUBLE, d_min, l_d_min, 1, false);
    CHECK(LONG_DOUBLE, l_d_min, d_min, 1, false);
    CHECK(LONG_DOUBLE, d_min, l_d_min_half, 0, false);
    CHECK(LONG_DOUBLE, l_d_min_half, d_min, 0, false);
    CHECK(LONG_DOUBLE, d_zero, l_d_min_half, 0, false);
    CHECK(LONG_DOUBLE, l_d_min_half, d_zero, 0, false);
    CHECK(LONG_DOUBLE, d_zero, l_min, 0, false);
    CHECK(LONG_DOUBLE, l_min, d_zero, 0, false);
    CHECK(LONG_DOUBLE, f_zero, l_min, 0, false);
    CHECK(LONG_DOUBLE, l_min, f_zero, 0, false);
#endif
    CHECK(LONG_DOUBLE, f_one, l_qnan, 0, true);
    CHECK(LONG_DOUBLE, l_qnan, d_one, 0, true);
    CHECK(LONG_DOUBLE, l_one, f_qnan, 0, true);
    CHECK(LONG_DOUBLE, d_qnan, l_one, 0, true);
    CHECK(LONG_DOUBLE, d_qnan, l_qnan, 0, true);
    CHECK(LONG_DOUBLE, l_one, d_snan, 0, true);
    CHECK(LONG_DOUBLE, f_snan, l_one, 0, true);
    CHECK(LONG_DOUBLE, d_one, l_snan, 0, true);
}
#endif

int
main(void)
{
    test_float();
    test_double();
    test_float_double();
#ifdef _TEST_LONG_DOUBLE
    test_long_double();
    test_mixed_long_double();
#endif
    printf("%d failures\n", failures);
    return failures != 0;
}
