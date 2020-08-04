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

#include <fenv.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

volatile float zero = 0.0f;
volatile float one = 1.0f;
volatile float two = 2.0f;
volatile float big = 3e38f;

typedef float (*test_t)(void);

#define cat2(a,b) a ## b
#define str(a) #a
#define TEST(n,v,ex,er)	{ .func = cat2(test_, n), .name = str(n), .value = (v), .except = (ex), .errno = (er) }

float test_acos_2(void) { return acos(two); }
float test_acosh_half(void) { return acosh(one/two); }
float test_asin_2(void) { return asin(two); }
float test_exp_big(void) { return expf(big); }
float test_exp_neg(void) { return expf(-big); }
float test_gamma_negi(void) { return gammaf(-one); }
float test_gamma_big(void) { return gammaf(big); }
float test_hypot_big(void) { return hypotf(big, big); }
float test_lgamma_negi(void) { return lgammaf(-one); }
float test_lgamma_big(void) { return lgammaf(big); }
float test_log_0(void) { return logf(zero); }
float test_log_neg(void) { return logf(-one); }
float test_log10_0(void) { return log10f(zero); }
float test_log10_neg(void) { return log10f(-one); }
float test_pow_neg_real(void) { return powf(-two, one/two); }
float test_pow_zero_neg(void) { return powf(zero, -two); }
float test_pow_big(void) { return powf(two, big); }
float test_pow_tiny(void) { return powf(two, -big); }
float test_remainder_zero(void) { return remainderf(two, zero); }
float test_sqrt_neg(void) { return sqrtf(-two); }
float test_y0_0(void) { return y0f(zero); }
float test_y0_neg(void) { return y0f(-one); }
float test_y1_0(void) { return y1f(zero); }
float test_y1_neg(void) { return y1f(-one); }
float test_yn_0(void) { return ynf(2, zero); }
float test_yn_neg(void) { return ynf(2, -one); }

struct {
	test_t	func;
	char	*name;
	float	value;
	int	except;
	int	errno;
} tests[] = {
	TEST(acos_2, nan(""), FE_INVALID, EDOM),
	TEST(acosh_half, nan(""), FE_INVALID, EDOM),
	TEST(asin_2, nan(""), FE_INVALID, EDOM),
	TEST(exp_big, INFINITY, FE_OVERFLOW, ERANGE),
	TEST(exp_neg, 0.0f, FE_UNDERFLOW, ERANGE),
	TEST(hypot_big, INFINITY, FE_OVERFLOW, ERANGE),
	TEST(gamma_negi, INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(gamma_big, INFINITY, FE_OVERFLOW, ERANGE),
	TEST(lgamma_negi, INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_big, INFINITY, FE_OVERFLOW, ERANGE),
	TEST(log_0, -INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log_neg, nan(""), FE_INVALID, EDOM),
	TEST(log10_0, -INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log10_neg, nan(""), FE_INVALID, EDOM),
	TEST(pow_neg_real, nan(""), FE_INVALID, EDOM),
	TEST(pow_zero_neg, INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(pow_big, INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow_tiny, 0.0f, FE_UNDERFLOW, ERANGE),
	TEST(remainder_zero, nan(""), FE_INVALID, EDOM),
	TEST(sqrt_neg, nan(""), FE_INVALID, EDOM),
	TEST(y0_0, -INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(y0_neg, nan(""), FE_INVALID, EDOM),
	TEST(y1_0, -INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(y1_neg, nan(""), FE_INVALID, EDOM),
	TEST(yn_0, -INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(yn_neg, nan(""), FE_INVALID, EDOM),
};

#define NTEST (sizeof(tests) / sizeof(tests[0]))

int main()
{
	int result = 0;
	volatile float v;
	int err, except;
	int t;

	for (t = 0; t < NTEST; t++) {
		errno = 0;
		feclearexcept(FE_ALL_EXCEPT);
		v = tests[t].func();
		err = errno;
		except = fetestexcept(FE_ALL_EXCEPT);
#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
		printf("%-20.20s = %g errno %d (%s) except 0x%x\n",
		       tests[t].name, v, err, strerror(err), except);
#else
		printf("%-20.20s = (float) errno %d (%s) except 0x%x\n",
		       tests[t].name, err, strerror(err), except);
#endif
		if ((isinf(v) && isinf(tests[t].value) && ((v > 0) != (tests[t].value > 0))) ||
		    (v != tests[t].value && isnanf(v) != isnanf(tests[t].value)))
		{
			printf("\tbad value got %g expect %g\n", v, tests[t].value);
			++result;
		}
		if (math_errhandling & MATH_ERREXCEPT) {
			if (!(except & tests[t].except)) {
				printf("\texceptions supported but %s returns 0x%x\n", tests[t].name, except);
				++result;
			}
		} else {
			if (except) {
				printf("\texceptions not supported but %s returns 0x%x\n", tests[t].name, except);
				++result;
			}
		}
		if (math_errhandling & MATH_ERRNO) {
			if (err != tests[t].errno) {
				printf("\terrno supported but %s returns %d (%s)\n", tests[t].name, err, strerror(err));
				++result;
			}
		} else {
			if (err != 0) {
				printf("\terrno not supported but %s returns %d (%s)\n", tests[t].name, err, strerror(err));
				++result;
			}
		}
	}
	return result;
}
