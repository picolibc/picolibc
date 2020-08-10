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

volatile FLOAT_T makemathname(zero) = 0.0;
volatile FLOAT_T makemathname(one) = 1.0;
volatile FLOAT_T makemathname(two) = 2.0;
volatile FLOAT_T makemathname(big) = BIG;

#define cat2(a,b) a ## b
#define str(a) #a
#define TEST(n,v,ex,er)	{ .func = makemathname(cat2(test_, n)), .name = str(n), .value = (v), .except = (ex), .errno = (er) }

FLOAT_T makemathname(test_acos_2)(void) { return makemathname(acos)(makemathname(two)); }
FLOAT_T makemathname(test_acosh_half)(void) { return makemathname(acosh)(makemathname(one)/makemathname(two)); }
FLOAT_T makemathname(test_asin_2)(void) { return makemathname(asin)(makemathname(two)); }
FLOAT_T makemathname(test_exp_big)(void) { return makemathname(exp)(makemathname(big)); }
FLOAT_T makemathname(test_exp_neg)(void) { return makemathname(exp)(-makemathname(big)); }
FLOAT_T makemathname(test_gamma_negi)(void) { return makemathname(gamma)(-makemathname(one)); }
FLOAT_T makemathname(test_gamma_big)(void) { return makemathname(gamma)(makemathname(big)); }
FLOAT_T makemathname(test_hypot_big)(void) { return makemathname(hypot)(makemathname(big), makemathname(big)); }
FLOAT_T makemathname(test_lgamma_negi)(void) { return makemathname(lgamma)(-makemathname(one)); }
FLOAT_T makemathname(test_lgamma_big)(void) { return makemathname(lgamma)(makemathname(big)); }
FLOAT_T makemathname(test_log_0)(void) { return makemathname(log)(makemathname(zero)); }
FLOAT_T makemathname(test_log_neg)(void) { return makemathname(log)(-makemathname(one)); }
FLOAT_T makemathname(test_log10_0)(void) { return makemathname(log10)(makemathname(zero)); }
FLOAT_T makemathname(test_log10_neg)(void) { return makemathname(log10)(-makemathname(one)); }
FLOAT_T makemathname(test_pow_neg_real)(void) { return makemathname(pow)(-makemathname(two), makemathname(one)/makemathname(two)); }
FLOAT_T makemathname(test_pow_0_neg)(void) { return makemathname(pow)(makemathname(zero), -makemathname(two)); }
FLOAT_T makemathname(test_pow_big)(void) { return makemathname(pow)(makemathname(two), makemathname(big)); }
FLOAT_T makemathname(test_pow_tiny)(void) { return makemathname(pow)(makemathname(two), -makemathname(big)); }
FLOAT_T makemathname(test_remainder_0)(void) { return makemathname(remainder)(makemathname(two), makemathname(zero)); }
FLOAT_T makemathname(test_sqrt_neg)(void) { return makemathname(sqrt)(-makemathname(two)); }
FLOAT_T makemathname(test_y0_0)(void) { return makemathname(y0)(makemathname(zero)); }
FLOAT_T makemathname(test_y0_neg)(void) { return makemathname(y0)(-makemathname(one)); }
FLOAT_T makemathname(test_y1_0)(void) { return makemathname(y1)(makemathname(zero)); }
FLOAT_T makemathname(test_y1_neg)(void) { return makemathname(y1)(-makemathname(one)); }
FLOAT_T makemathname(test_yn_0)(void) { return makemathname(yn)(2, makemathname(zero)); }
FLOAT_T makemathname(test_yn_neg)(void) { return makemathname(yn)(2, -makemathname(one)); }

struct {
	FLOAT_T	(*func)(void);
	char	*name;
	FLOAT_T	value;
	int	except;
	int	errno;
} makemathname(tests)[] = {
	TEST(acos_2, NAN, FE_INVALID, EDOM),
	TEST(acosh_half, NAN, FE_INVALID, EDOM),
	TEST(asin_2, NAN, FE_INVALID, EDOM),
	TEST(exp_big, INFINITY, FE_OVERFLOW, ERANGE),
	TEST(exp_neg, 0.0f, FE_UNDERFLOW, ERANGE),
	TEST(hypot_big, INFINITY, FE_OVERFLOW, ERANGE),
	TEST(gamma_negi, INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(gamma_big, INFINITY, FE_OVERFLOW, ERANGE),
	TEST(lgamma_negi, INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_big, INFINITY, FE_OVERFLOW, ERANGE),
	TEST(log_0, -INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log_neg, NAN, FE_INVALID, EDOM),
	TEST(log10_0, -INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log10_neg, NAN, FE_INVALID, EDOM),
	TEST(pow_neg_real, NAN, FE_INVALID, EDOM),
	TEST(pow_0_neg, INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(pow_big, INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow_tiny, 0.0f, FE_UNDERFLOW, ERANGE),
	TEST(remainder_0, NAN, FE_INVALID, EDOM),
	TEST(sqrt_neg, NAN, FE_INVALID, EDOM),
	TEST(y0_0, -INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(y0_neg, NAN, FE_INVALID, EDOM),
	TEST(y1_0, -INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(y1_neg, NAN, FE_INVALID, EDOM),
	TEST(yn_0, -INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(yn_neg, NAN, FE_INVALID, EDOM),
	{ NULL, NULL },
};

int
makemathname(run_tests)(void) {
	int result = 0;
	volatile float v;
	int err, except;
	int t;

	for (t = 0; makemathname(tests)[t].func; t++) {
		errno = 0;
		feclearexcept(FE_ALL_EXCEPT);
		v = makemathname(tests)[t].func();
		err = errno;
		except = fetestexcept(FE_ALL_EXCEPT);
#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
		printf("    %-20.20s = %g errno %d (%s) except 0x%x\n",
		       makemathname(tests)[t].name, v, err, strerror(err), except);
#else
		printf("    %-20.20s = (float) errno %d (%s) except 0x%x\n",
		       makemathname(tests)[t].name, err, strerror(err), except);
#endif
		if ((isinf(v) && isinf(makemathname(tests)[t].value) && ((v > 0) != (makemathname(tests)[t].value > 0))) ||
		    (v != makemathname(tests)[t].value && isnanf(v) != isnanf(makemathname(tests)[t].value)))
		{
			printf("\tbad value got %g expect %g\n", v, makemathname(tests)[t].value);
			++result;
		}
		if (math_errhandling & EXCEPTION_TEST) {
			if (!(except & makemathname(tests)[t].except)) {
				printf("\texceptions supported but %s returns 0x%x\n", makemathname(tests)[t].name, except);
				++result;
			}
		} else {
			if (except) {
				printf("\texceptions not supported but %s returns 0x%x\n", makemathname(tests)[t].name, except);
				++result;
			}
		}
		if (math_errhandling & MATH_ERRNO) {
			if (err != makemathname(tests)[t].errno) {
				printf("\terrno supported but %s returns %d (%s)\n", makemathname(tests)[t].name, err, strerror(err));
				++result;
			}
		} else {
			if (err != 0) {
				printf("\terrno not supported but %s returns %d (%s)\n", makemathname(tests)[t].name, err, strerror(err));
				++result;
			}
		}
	}
	return result;
}
