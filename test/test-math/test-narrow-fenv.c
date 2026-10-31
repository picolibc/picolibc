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
 * Check rounding modes and exceptions for the narrowing functions,
 * which compute in a wider type and round once to the narrow type.
 * test-narrow-fenv.h holds the checks and is included once for each
 * family of functions.
 */

#define _ISOC23_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include <fenv.h>

#ifdef __sh__
#if !(defined(__SH4__) || defined(__SH4_SINGLE__) || defined(__SH4_SINGLE_ONLY__))
#define GDB_SIMULATOR
#endif
#endif

#ifndef FE_INEXACT
#define FE_INEXACT 0
#endif
#ifndef FE_UNDERFLOW
#define FE_UNDERFLOW 0
#endif
#ifndef FE_OVERFLOW
#define FE_OVERFLOW 0
#endif
#ifndef FE_INVALID
#define FE_INVALID 0
#endif
#ifndef FE_DIVBYZERO
#define FE_DIVBYZERO 0
#endif

#define MY_EXCEPT (FE_INEXACT | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID | FE_DIVBYZERO)

/* The long double narrowing functions only exist with long double math */
#if defined(_TEST_LONG_DOUBLE) && defined(__HAVE_LONG_DOUBLE_MATH)
#define TEST_LONG_DOUBLE
#endif

static const struct {
    int         mode;
    const char *name;
} modes[] = {
#ifdef FE_TONEAREST
    { FE_TONEAREST,  "FE_TONEAREST"  },
#endif
#ifdef FE_UPWARD
    { FE_UPWARD,     "FE_UPWARD"     },
#endif
#ifdef FE_DOWNWARD
    { FE_DOWNWARD,   "FE_DOWNWARD"   },
#endif
#ifdef FE_TOWARDZERO
    { FE_TOWARDZERO, "FE_TOWARDZERO" },
#endif
};

#define NMODES (sizeof(modes) / sizeof(modes[0]))

/* Expected result for each rounding mode */
typedef struct {
    double nearest, upward, downward, towardzero;
} expect_t;

static double
expected(const expect_t *e, int mode)
{
    switch (mode) {
#ifdef FE_UPWARD
    case FE_UPWARD:
        return e->upward;
#endif
#ifdef FE_DOWNWARD
    case FE_DOWNWARD:
        return e->downward;
#endif
#ifdef FE_TOWARDZERO
    case FE_TOWARDZERO:
        return e->towardzero;
#endif
    default:
        return e->nearest;
    }
}

static bool
same(double a, double b)
{
    if (isnan(a) || isnan(b))
        return isnan(a) && isnan(b);
    return a == b && !signbit(a) == !signbit(b);
}

#ifdef TEST_LONG_DOUBLE
/*
 * m68k soft-float emulates long double with double, which can't
 * represent the long double limits these tests use
 */
static bool
skip_long_double(void)
{
#ifdef __m68k__
    volatile long double zero = 0.0L;
    volatile long double one = 1.0L;
    volatile long double check = nextafterl(zero, one);
    if (check + check == zero) {
        printf("m68k emulating long double with double, skipping long double tests\n");
        return true;
    }
#endif
    return false;
}
#endif

/* double -> float */
#define N_T        float
#define W_T        double
#define FAMILY     "f*"
#define ADD        fadd
#define SUB        fsub
#define MUL        fmul
#define DIV        fdiv
#define FMA        ffma
#define SQRT       fsqrt
#define N_MAX      FLT_MAX
#define N_TRUE_MIN FLT_TRUE_MIN
#define N_EPSILON  FLT_EPSILON
#define W_MAX      DBL_MAX
#define W_TRUE_MIN DBL_TRUE_MIN
#define N_MANT_DIG FLT_MANT_DIG
#define W_MANT_DIG DBL_MANT_DIG
#define NEXTUP_N   nextupf
#if defined(__FLOAT_NOROUND) || defined(__DOUBLE_NOROUND)
#define NO_ROUND
#endif
#if defined(__FLOAT_NOEXCEPT) || defined(__DOUBLE_NOEXCEPT)
#define NO_EXCEPT
#endif
#define SKIP_FAMILY false
#define test_family test_f
#include "test-narrow-fenv.h"

#ifdef TEST_LONG_DOUBLE
/* long double -> float */
#define N_T        float
#define W_T        long double
#define FAMILY     "f*l"
#define ADD        faddl
#define SUB        fsubl
#define MUL        fmull
#define DIV        fdivl
#define FMA        ffmal
#define SQRT       fsqrtl
#define N_MAX      FLT_MAX
#define N_TRUE_MIN FLT_TRUE_MIN
#define N_EPSILON  FLT_EPSILON
#define W_MAX      LDBL_MAX
#define W_TRUE_MIN LDBL_TRUE_MIN
#define N_MANT_DIG FLT_MANT_DIG
#define W_MANT_DIG LDBL_MANT_DIG
#define NEXTUP_N   nextupf
#if defined(__FLOAT_NOROUND) || defined(__LONG_DOUBLE_NOROUND)
#define NO_ROUND
#endif
#if defined(__FLOAT_NOEXCEPT) || defined(__LONG_DOUBLE_NOEXCEPT)
#define NO_EXCEPT
#endif
#define SKIP_FAMILY skip_long_double()
#define test_family test_fl
#include "test-narrow-fenv.h"

/* long double -> double */
#define N_T        double
#define W_T        long double
#define FAMILY     "d*l"
#define ADD        daddl
#define SUB        dsubl
#define MUL        dmull
#define DIV        ddivl
#define FMA        dfmal
#define SQRT       dsqrtl
#define N_MAX      DBL_MAX
#define N_TRUE_MIN DBL_TRUE_MIN
#define N_EPSILON  DBL_EPSILON
#define W_MAX      LDBL_MAX
#define W_TRUE_MIN LDBL_TRUE_MIN
#define N_MANT_DIG DBL_MANT_DIG
#define W_MANT_DIG LDBL_MANT_DIG
#define NEXTUP_N   nextup
#if defined(__DOUBLE_NOROUND) || defined(__LONG_DOUBLE_NOROUND)
#define NO_ROUND
#endif
#if defined(__DOUBLE_NOEXCEPT) || defined(__LONG_DOUBLE_NOEXCEPT)
#define NO_EXCEPT
#endif
#define SKIP_FAMILY skip_long_double()
#define test_family test_dl
#include "test-narrow-fenv.h"
#endif

int
main(void)
{
    int ret = 0;

#ifdef GDB_SIMULATOR
    printf("GDB simulator doesn't support rounding modes or exceptions. Skipping\n");
    return 77;
#endif
    ret += test_f();
#ifdef TEST_LONG_DOUBLE
    ret += test_fl();
    ret += test_dl();
#endif
    if (ret)
        printf("%d failures\n", ret);
    return ret != 0;
}
