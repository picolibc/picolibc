/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Keith Packard
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

#include <stdio.h>
#include <math.h>
#include <fenv.h>

#ifndef PICOLIBC_DOUBLE_NOROUND
static double
do_round_int(double value, int mode)
{
	switch (mode) {
#ifdef FE_TONEAREST
	case FE_TONEAREST:
		return round(value);
#endif
#ifdef FE_UPWARD
	case FE_UPWARD:
		return ceil(value);
#endif
#ifdef FE_DOWNWARD
	case FE_DOWNWARD:
		return floor(value);
#endif
#ifdef FE_TOWARDZERO
	case FE_TOWARDZERO:
		return trunc(value);
#endif
	default:
		return value;
	}
}
#endif

#ifndef PICOLIBC_FLOAT_NOROUND
static float
do_roundf_int(float value, int mode)
{
	switch (mode) {
#ifdef FE_TONEAREST
	case FE_TONEAREST:
		return roundf(value);
#endif
#ifdef FE_UPWARD
	case FE_UPWARD:
		return ceilf(value);
#endif
#ifdef FE_DOWNWARD
	case FE_DOWNWARD:
		return floorf(value);
#endif
#ifdef FE_TOWARDZERO
	case FE_TOWARDZERO:
		return truncf(value);
#endif
	default:
		return value;
	}
}
#endif

double
div(double a, double b);

double
mul(double a, double b);

double
sub(double a, double b);

float
div_f(float a, float b);

float
mul_f(float a, float b);

float
sub_f(float a, float b);

double
div_mul_sub(double a, double b, double c, double d)
{
	return sub(mul(div(a,b), c), d);
}

#define do_fancy(sign) div_mul_sub(sign 1.0, 3.0, 3.0, sign 1.0)

float
div_f_mul_sub(float a, float b, float c, float d)
{
	return sub_f(mul_f(div_f(a,b), c), d);
}

#define do_fancyf(sign) div_f_mul_sub(sign 1.0f, 3.0f, 3.0f, sign 1.0f)

#define n4      nextafter(4.0, 5.0)
#define nn4     nextafter(nextafter(4.0, 5.0), 5.0)
#define n2      nextafter(2.0, 3.0)
#define nn2     nextafter(nextafter(2.0, 3.0), 3.0)

#define n4f     nextafterf(4.0f, 5.0f)
#define nn4f    nextafterf(nextafterf(4.0f, 5.0f), 5.0f)
#define n2f     nextafterf(2.0f, 3.0f)

static int
check(int mode, char *name, double value)
{
	int ret = 0;

        (void) mode;
        (void) name;
        (void) value;
#ifndef PICOLIBC_DOUBLE_NOROUND
	double want, got;
	printf("test double %s for value %g \n", name, value);
	want = do_round_int(value, mode);
	if (fesetround(mode) != 0) {
		printf("ERROR fesetround %s failed\n", name);
		ret++;
	}
	got = nearbyint(value);
	if (want != got) {
		printf("ERROR double %s: value %g want %g got %g\n", name, value, want, got);
		ret++;
	}

	want = do_round_int(-value, mode);
	if (fesetround(mode) != 0) {
		printf("ERROR fesetround %s failed\n", name);
		ret++;
	}
	got = nearbyint(-value);
	if (want != got) {
		printf("ERROR double %s: -value %g want %g got %g\n", name, value, want, got);
		ret++;
	}
#endif
#ifndef PICOLIBC_FLOAT_NOROUND
	float valuef = value, wantf, gotf;

	printf("test float %s for value %g \n", name, value);
	wantf = do_roundf_int(valuef, mode);
	if (fesetround(mode) != 0) {
		printf("ERROR fesetround %s failed\n", name);
		ret++;
	}
	gotf = nearbyintf(valuef);
	if (wantf != gotf) {
		printf("ERROR float %s: value %g want %g got %g\n", name, (double) valuef, (double) wantf, (double) gotf);
		ret++;
	}

	wantf = do_roundf_int(-valuef, mode);
	if (fesetround(mode) != 0) {
		printf("ERROR fesetround %s failed\n", name);
		ret++;
	}
	gotf = nearbyintf(-valuef);
	if (wantf != gotf) {
		printf("ERROR float %s: -value %g want %g got %g\n", name, (double) valuef, (double) wantf, (double) gotf);
		ret++;
	}
#endif
	if (ret)
		printf("ERROR %s failed\n", name);
	return ret;
}

static double my_values[] = {
	1.0,
	1.0 / 3.0,
	3.0 / 2.0,	/* let either round out or round even work */
	2.0 / 3.0,
};

#define NUM_VALUE (sizeof(my_values)/sizeof(my_values[0]))

int main(void)
{
	unsigned i;
	int ret = 0;

	for (i = 0; i < NUM_VALUE; i++) {
#ifdef FE_TONEAREST
		ret += check(FE_TONEAREST, "FE_TONEAREST", my_values[i]);
#endif
#ifdef FE_UPWARD
		ret += check(FE_UPWARD, "FE_UPWARD", my_values[i]);
#endif
#ifdef FE_DOWNWARD
		ret += check(FE_DOWNWARD, "FE_DOWNWARD", my_values[i]);
#endif
#ifdef FE_TOWARDZERO
		ret += check(FE_TOWARDZERO, "FE_TOWARDZERO", my_values[i]);
#endif
	}
#if defined(FE_UPWARD) && defined(FE_DOWNWARD) && defined(FE_TOWARDZERO)

#define check_func(big, small, name) do {				\
		printf("testing %s\n", name); \
		if (big <= small) {					\
			printf("ERROR %s: %g is not > %g\n", name, (double) big, (double) small); \
			ret++;						\
		}							\
		if (big < 0) {						\
			printf("ERROR %s: %g is not >= 0\n", name, (double) big); \
			ret++;						\
		}							\
		if (small > 0) {					\
			printf("ERROR %s: %g is not <= 0\n", name, (double) small); \
			ret++;						\
		}							\
	} while(0)


#ifndef PICOLIBC_DOUBLE_NOROUND
	double up_plus, toward_plus, down_plus, up_minus, toward_minus, down_minus;

	fesetround(FE_UPWARD);
	up_plus = do_fancy();
	up_minus = do_fancy(-);

	fesetround(FE_DOWNWARD);
	down_plus = do_fancy();
	down_minus = do_fancy(-);

	fesetround(FE_TOWARDZERO);
	toward_plus = do_fancy();
	toward_minus = do_fancy(-);

	check_func(up_plus, down_plus, "up/down");
	check_func(up_plus, toward_plus, "up/toward");
	check_func(up_minus, down_minus, "-up/-down");
	check_func(toward_minus, down_minus, "-toward/-down");

#define check_sqrt(mode, param, expect) do {                            \
                fesetround(mode);                                       \
                double __p = (param);                                   \
                double __e = (expect);                                  \
                printf("testing sqrt %s for value %a\n", #mode, __p);   \
                double __r = sqrt(__p);                                 \
                if (__r != __e) {                                       \
                        printf("ERROR %s: sqrt(%a) got %a expect %a\n", #mode, __p, __r, __e); \
                        ret++;                                          \
                }                                                       \
        } while(0)

        check_sqrt(FE_TONEAREST, n4, 2.0);
        check_sqrt(FE_TONEAREST, nn4, n2);

        check_sqrt(FE_UPWARD, n4, n2);
        check_sqrt(FE_UPWARD, nn4, n2);

        check_sqrt(FE_DOWNWARD, n4, 2.0);
        check_sqrt(FE_DOWNWARD, nn4, 2.0);

        check_sqrt(FE_TOWARDZERO, n4, 2.0);
        check_sqrt(FE_TOWARDZERO, nn4, 2.0);
#endif
#ifndef PICOLIBC_FLOAT_NOROUND
	float fup_plus, ftoward_plus, fdown_plus, fup_minus, ftoward_minus, fdown_minus;

	fesetround(FE_UPWARD);
	fup_plus = do_fancyf();
	fup_minus = do_fancyf(-);

	fesetround(FE_DOWNWARD);
	fdown_plus = do_fancyf();
	fdown_minus = do_fancyf(-);

	fesetround(FE_TOWARDZERO);
	ftoward_plus = do_fancyf();
	ftoward_minus = do_fancyf(-);

	check_func(fup_plus, fdown_plus, "fup/fdown");
	check_func(fup_plus, ftoward_plus, "fup/ftoward");
	check_func(fup_minus, fdown_minus, "-fup/-fdown");
	check_func(ftoward_minus, fdown_minus, "-ftoward/-fdown");

#define check_sqrtf(mode, param, expect) do {                           \
                fesetround(mode);                                       \
                float __p = (param);                                    \
                float __e = (expect);                                   \
                printf("testing sqrtf %s for value %a\n", #mode, (double) __p); \
                float __r = sqrtf(__p);                                 \
                if (__r != __e) {                                       \
                    printf("ERROR %s: sqrtf(%a) got %a expect %a\n", #mode, (double) __p, (double) __r, (double) __e); \
                        ret++;                                          \
                }                                                       \
        } while(0)

        check_sqrtf(FE_TONEAREST, n4f, 2.0f);
        check_sqrtf(FE_TONEAREST, nn4f, n2f);

        check_sqrtf(FE_UPWARD, n4f, n2f);
        check_sqrtf(FE_UPWARD, nn4f, n2f);

        check_sqrtf(FE_DOWNWARD, n4f, 2.0f);
        check_sqrtf(FE_DOWNWARD, nn4f, 2.0f);

        check_sqrtf(FE_TOWARDZERO, n4f, 2.0f);
        check_sqrtf(FE_TOWARDZERO, nn4f, 2.0f);

#endif
#endif

	return ret;
}
