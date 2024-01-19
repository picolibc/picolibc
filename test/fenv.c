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

#ifdef __STDC_IEC_559__
#define HAVE_HW_DOUBLE
#endif

#ifdef HAVE_HW_DOUBLE
typedef double test_t;
#define test_sqrt(x) sqrt(x)
#define test_pow(x,y) pow(x,y)
#define test_remainder(x,y) remainder(x,y)
#define test_log(x) log(x)
#define tiny_val 1e-300
#define huge_val 1e300;
#else
typedef float test_t;
#define test_sqrt(x) sqrtf(x)
#define test_pow(x,y) powf(x,y)
#define test_remainder(x,y) remainderf(x,y)
#define test_log(x) logf(x)
#define tiny_val 1e-30f
#define huge_val 1e30f
#endif

volatile test_t one = 1.0;
volatile test_t zero = 0.0;
volatile test_t two = 2.0;
volatile test_t huge = huge_val;
volatile test_t tiny = tiny_val;
volatile test_t inf = INFINITY;

#define lowbit(x) 	((x) & -(x))
#define ispoweroftwo(x)	(((x) & ((x) - 1)) == 0)

#ifdef FE_DIVBYZERO
#define my_divbyzero FE_DIVBYZERO
#else
#define my_divbyzero 0
#endif

#ifdef FE_OVERFLOW
#define my_overflow FE_OVERFLOW
#else
#define my_overflow 0
#endif

#ifdef FE_UNDERFLOW
#define my_underflow FE_UNDERFLOW
#else
#define my_underflow 0
#endif

#ifdef FE_INEXACT
#define my_inexact FE_INEXACT
#else
#define my_inexact 0
#endif

#ifdef FE_INVALID
#define my_invalid FE_INVALID
#else
#define my_invalid 0
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
	static char buf[50];
	sprintf(buf, "Invalid 0x%x", e);
	return buf;
}

#define s(e) #e

static int
report(char *expr, test_t v, int e, int exception, int oexception)
{
        /* powerpc has additional details in the exception flags */
        e &= (my_inexact | my_divbyzero | my_underflow | my_overflow | my_invalid);
	printf("%-20.20s: ", expr);
	printf("%8g ", (double) v);
	printf("(e expect %s", e_to_str(exception));
	if (oexception)
		printf(" or %s", e_to_str(oexception));
	printf(" got %s\n", e_to_str(e));
	if (e == (exception) ||
	    (oexception && e == (oexception)))
	{
		return 0;
	}
	printf("\tgot %s expecting %s", e_to_str(e), e_to_str(exception));
	if (oexception)
		printf(" or %s", e_to_str(oexception));
	printf("\n");
	return 1;
}

#define TEST_CASE2(expr, exception, oexception) do {			\
		int e;							\
		volatile test_t v;					\
		feclearexcept(FE_ALL_EXCEPT);				\
		v = expr;						\
		e = fetestexcept(FE_ALL_EXCEPT);			\
		result += report(s(expr), v, e, exception, oexception); \
	} while(0)

#define TEST_CASE(expr, exception) do {					\
		if ((exception & (my_overflow|my_underflow)) && my_inexact != 0) \
			TEST_CASE2(expr, exception, exception | my_inexact); \
		else							\
			TEST_CASE2(expr, exception, 0);			\
	} while(0)

static const struct {
    const char *name;
    int value;
} excepts[] = {
    { .name = "None", .value = 0 },
#if FE_DIVBYZERO
    { .name = "Divide by zero", .value = FE_DIVBYZERO },
#endif
#if FE_OVERFLOW
    { .name = "Overflow", .value = FE_OVERFLOW },
#endif
#if FE_UNDERFLOW
    { .name = "Underflow", .value = FE_UNDERFLOW },
#endif
#if FE_INVALID
    { .name = "Invalid", .value = FE_INVALID },
#endif
};

#define NUM_EXCEPTS (sizeof(excepts)/sizeof(excepts[0]))

int main(void)
{
	int result = 0;
        int ret;
        unsigned i;

	(void) report;
	(void) e_to_str;
	if (math_errhandling & MATH_ERREXCEPT) {
#if FE_DIVBYZERO
		TEST_CASE(one / zero, FE_DIVBYZERO);
		TEST_CASE(test_log(zero), FE_DIVBYZERO);
#endif
#if FE_OVERFLOW
		TEST_CASE(huge * huge, FE_OVERFLOW);
		TEST_CASE(test_pow(two, huge), FE_OVERFLOW);
#endif
#if FE_UNDERFLOW
		TEST_CASE(tiny * tiny, FE_UNDERFLOW);
		TEST_CASE(test_pow(two, -huge), FE_UNDERFLOW);
#endif
#if FE_INVALID
		TEST_CASE(zero * inf, FE_INVALID);
		TEST_CASE(inf * zero, FE_INVALID);
		TEST_CASE(inf + -inf, FE_INVALID);
		TEST_CASE(inf - inf, FE_INVALID);
		TEST_CASE(zero / zero, FE_INVALID);
		TEST_CASE(inf / inf, FE_INVALID);
		TEST_CASE(test_remainder(one, zero), FE_INVALID);
		TEST_CASE(test_remainder(inf, two), FE_INVALID);
		TEST_CASE(test_sqrt(-two), FE_INVALID);
#endif
	}

        for (i = 0; i < NUM_EXCEPTS; i++) {
            ret = feenableexcept(excepts[i].value);
            if (ret == 0) {
                ret = fedisableexcept(excepts[i].value);
                if (ret != excepts[i].value) {
                    printf("enable %s worked, disabled returned %d\n", excepts[i].name, ret);
                    result = 1;
                }
            } else {
                if (excepts[i].value == 0) {
                    printf("enable %s returned %d", excepts[i].name, ret);
                    result = 1;
                }
            }
        }
	return result;
}
