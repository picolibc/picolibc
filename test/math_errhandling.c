/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
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
#include <fenv.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>

#ifdef _HAVE_ATTRIBUTE_ALWAYS_INLINE
#define ALWAYS_INLINE __inline__ __attribute__((__always_inline__))
#else
#define ALWAYS_INLINE __inline__
#endif

static ALWAYS_INLINE float
opt_barrier_float (float x)
{
  volatile float y = x;
  return y;
}

static ALWAYS_INLINE double
opt_barrier_double (double x)
{
  volatile double y = x;
  return y;
}

#if __SIZEOF_LONG_DOUBLE__ > 0
static ALWAYS_INLINE long double
opt_barrier_long_double (long double x)
{
  volatile long double y = x;
  return y;
}
#endif

#ifdef __clang__
#define clang_barrier_long_double(x) opt_barrier_long_double(x)
#define clang_barrier_double(x) opt_barrier_double(x)
#define clang_barrier_float(x) opt_barrier_float(x)
#else
#define clang_barrier_long_double(x) (x)
#define clang_barrier_double(x) (x)
#define clang_barrier_float(x) (x)
#endif

static const char *
e_to_str(int e)
{
	if (e == 0)
		return "NONE";

#ifdef FE_DIVBYZERO
	if (e == FE_DIVBYZERO)
		return "FE_DIVBYZERO";
#endif
#ifdef FE_OVERFLOW
	if (e == FE_OVERFLOW)
		return "FE_OVERFLOW";
#endif
#ifdef FE_UNDERFLOW
	if (e == FE_UNDERFLOW)
		return "FE_UNDERFLOW";
#endif
#ifdef FE_INEXACT
	if (e == FE_INEXACT)
		return "FE_INEXACT";
#endif
#ifdef FE_INVALID
	if (e == FE_INVALID)
		return "FE_INVALID";
#endif
#if defined(FE_OVERFLOW) && defined(FE_INEXACT)
	if (e == (FE_OVERFLOW|FE_INEXACT))
		return "FE_OVERFLOW|FE_INEXACT";
#endif
#if defined(FE_UNDERFLOW) && defined(FE_INEXACT)
	if (e == (FE_UNDERFLOW|FE_INEXACT))
		return "FE_UNDERFLOW|FE_INEXACT";
#endif
	static char buf[3][128];
        static int i = 0;
        buf[i][0] = '\0';
        while (e) {
            char *v = NULL;
            char tmp[24];
#ifdef FE_DIVBYZERO
            if (e & FE_DIVBYZERO) {
		v = "FE_DIVBYZERO";
                e &= ~FE_DIVBYZERO;
            } else
#endif
#ifdef FE_OVERFLOW
            if (e & FE_OVERFLOW) {
		v = "FE_OVERFLOW";
                e &= ~FE_OVERFLOW;
            } else
#endif
#ifdef FE_UNDERFLOW
            if (e & FE_UNDERFLOW) {
		v = "FE_UNDERFLOW";
                e &= ~FE_UNDERFLOW;
            } else
#endif
#ifdef FE_INEXACT
            if (e & FE_INEXACT) {
		v = "FE_INEXACT";
                e &= ~FE_INEXACT;
            } else
#endif
#ifdef FE_INVALID
            if (e & FE_INVALID) {
		v = "FE_INVALID";
                e &= ~FE_INVALID;
            } else
#endif
            {
                snprintf(tmp, sizeof(tmp), "?? 0x%x", e);
                v = tmp;
                e = 0;
            }
#define check_add(s) do {                                               \
                if (strlen(buf[i]) + strlen(s) + 1 > sizeof(buf[i]))    \
                    printf("exception buf overflow %s + %s\n", buf[i], s); \
                else                                                    \
                    strcat(buf[i], s);                                  \
            } while(0)
            if (buf[i][0])
                check_add(" | ");
            check_add(v);
        }
        char *ret = buf[i];
        i = (i + 1) % 3;
        return ret;
}

#define scat(a,b) a ## b

#ifdef __mcffpu__
#define SKIP_SNAN_CHECKS
#endif

#ifdef __HAVE_68881__
#define SKIP_SNAN_CHECKS
#endif

#if defined(__m68k__) && !defined(__mcoldfire__) && !defined(__HAVE_M68881__)
#undef _TEST_LONG_DOUBLE
#define NO_NEXTTOWARD
#endif

/* Tests with long doubles */
#ifdef _TEST_LONG_DOUBLE

#if defined(__PICOLIBC__) && !defined(_HAVE_LONG_DOUBLE_MATH)
#define SIMPLE_MATH_ONLY
#define NO_NEXTTOWARD
#endif

#ifdef PICOLIBC_LONG_DOUBLE_NOEXCEPT
#define EXCEPTION_TEST 0
#else
#define EXCEPTION_TEST	MATH_ERREXCEPT
#endif
#define LONG_DOUBLE_EXCEPTION_TEST EXCEPTION_TEST
#ifdef _M_PI_L
#define PI_VAL _M_PI_L
#else
#define PI_VAL  3.141592653589793238462643383279502884L
#endif

#ifdef __PICOLIBC__
#define NO_BESSEL_TESTS
#endif

#define _PASTE_LDBL(exp) 0x.fp ## exp ## L
#define PASTE_LDBL(exp) _PASTE_LDBL(exp)

#define BIG PASTE_LDBL(__LDBL_MAX_EXP__)
#if __LDBL_MANT_DIG__ >= 64
#define BIGODD  0x1.123456789abcdef2p+63l
#define BIGEVEN 0x1.123456789abcdef0p+63l
#else
#define BIGODD  0x1.123456789abcdp+52l
#define BIGEVEN 0x1.123456789abccp+52l
#endif
#define SMALL __LDBL_DENORM_MIN__
#define FLOAT_T long double
#define MIN_VAL __LDBL_DENORM_MIN__
#define MAX_VAL __LDBL_MAX__
#define MANT_DIG __LDBL_MANT_DIG__
#define EPSILON_VAL __LDBL_EPSILON__
#define sNAN __builtin_nansl("")
#define force_eval(x) clang_barrier_long_double(x)

#define TEST_LONG_DOUBLE

#define makemathname(s) scat(s,l)
#define makemathname_r(s) scat(s,l_r)

#include "math_errhandling_tests.c"

#undef BIG
#undef BIGODD
#undef BIGEVEN
#undef SMALL
#undef MIN_VAL
#undef MAX_VAL
#undef MANT_DIG
#undef EPSILON_VAL
#undef force_eval
#undef sNAN
#undef makemathname
#undef makemathname_r
#undef FLOAT_T
#undef EXCEPTION_TEST
#undef TEST_LONG_DOUBLE
#undef NO_BESSEL_TESTS
#undef PI_VAL
#undef SIMPLE_MATH_ONLY
#endif

/* Tests with doubles */
#ifdef PICOLIBC_DOUBLE_NOEXCEPT
#define EXCEPTION_TEST 0
#else
#define EXCEPTION_TEST	MATH_ERREXCEPT
#endif
#define DOUBLE_EXCEPTION_TEST EXCEPTION_TEST

#if __SIZEOF_DOUBLE__ == 4
#define BIG 3e38
#define BIGODD  0x1.123456p+23
#define BIGEVEN 0x1.123454p+23
#define SMALL 1e-45
#else
#define BIG 1.7e308
#define BIGODD  0x1.123456789abcdp+52
#define BIGEVEN 0x1.123456789abccp+52
#define SMALL 5e-324
#endif
#define FLOAT_T double
#define MIN_VAL __DBL_DENORM_MIN__
#define MAX_VAL __DBL_MAX__
#define MANT_DIG __DBL_MANT_DIG__
#define EPSILON_VAL __DBL_EPSILON__
#define sNAN __builtin_nans("")
#define force_eval(x) clang_barrier_double(x)
#define PI_VAL M_PI

#define TEST_DOUBLE

#define makemathname(s) s
#define makemathname_r(s) scat(s,_r)

#include "math_errhandling_tests.c"

#undef BIG
#undef BIGODD
#undef BIGEVEN
#undef SMALL
#undef MIN_VAL
#undef MAX_VAL
#undef MANT_DIG
#undef EPSILON_VAL
#undef force_eval
#undef sNAN
#undef makemathname
#undef makemathname_r
#undef FLOAT_T
#undef EXCEPTION_TEST
#undef TEST_DOUBLE
#undef PI_VAL

/* Tests with floats */
#define EXCEPTION_TEST	MATH_ERREXCEPT
#define BIG 3e38
#define BIGODD  0x1.123456p+23
#define BIGEVEN 0x1.123454p+23
#define SMALL 1e-45
#define MIN_VAL 0x8p-152f
#define MAX_VAL 0xf.fffffp+124f
#define MANT_DIG __FLT_MANT_DIG__
#define EPSILON_VAL 0x1p-23f
#define sNAN __builtin_nansf("")
#define force_eval(x) clang_barrier_float(x)
#define FLOAT_T float
#define TEST_FLOAT
#define makemathname(s) scat(s,f)
#define makemathname_r(s) scat(s,f_r)
#define PI_VAL ((float) M_PI)

#include "math_errhandling_tests.c"

int main(void)
{
	int result = 0;

	printf("Double tests:\n");
	result += run_tests();
#ifdef _TEST_LONG_DOUBLE
	printf("Long double tests:\n");
#ifdef __m68k__
        volatile long double zero = 0.0L;
        volatile long double one = 1.0L;
        volatile long double check = nextafterl(zero, one);
        if (check + check == zero) {
            printf("m68k emulating long double with double, skipping\n");
        } else
#endif
	result += run_testsl();
#endif
	printf("Float tests:\n");
	result += run_testsf();
	return result;
}
