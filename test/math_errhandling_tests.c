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
volatile FLOAT_T makemathname(negzero) = -0.0;
volatile FLOAT_T makemathname(one) = 1.0;
volatile FLOAT_T makemathname(two) = 2.0;
volatile FLOAT_T makemathname(big) = BIG;
volatile FLOAT_T makemathname(small) = SMALL;

#define cat2(a,b) a ## b
#define str(a) #a
#define TEST(n,v,ex,er)	{ .func = makemathname(cat2(test_, n)), .name = str(n), .value = (v), .except = (ex), .errno_expect = (er) }

static int _signgam;

FLOAT_T makemathname(test_acos_2)(void) { return makemathname(acos)(makemathname(two)); }
FLOAT_T makemathname(test_acosh_half)(void) { return makemathname(acosh)(makemathname(one)/makemathname(two)); }
FLOAT_T makemathname(test_asin_2)(void) { return makemathname(asin)(makemathname(two)); }
FLOAT_T makemathname(test_exp_big)(void) { return makemathname(exp)(makemathname(big)); }
FLOAT_T makemathname(test_exp_neg)(void) { return makemathname(exp)(-makemathname(big)); }
FLOAT_T makemathname(test_tgamma_0)(void) { return makemathname(tgamma)(makemathname(zero)); }
FLOAT_T makemathname(test_tgamma_neg0)(void) { return makemathname(tgamma)(makemathname(negzero)); }
FLOAT_T makemathname(test_tgamma_neg1)(void) { return makemathname(tgamma)(-makemathname(one)); }
FLOAT_T makemathname(test_tgamma_big)(void) { return makemathname(tgamma)(makemathname(big)); }
FLOAT_T makemathname(test_tgamma_negbig)(void) { return makemathname(tgamma)(makemathname(-big)); }
FLOAT_T makemathname(test_tgamma_inf)(void) { return makemathname(tgamma)((FLOAT_T) INFINITY); }
FLOAT_T makemathname(test_tgamma_neginf)(void) { return makemathname(tgamma)((FLOAT_T) -INFINITY); }
FLOAT_T makemathname(test_tgamma_small)(void) { return makemathname(tgamma)(SMALL); }
FLOAT_T makemathname(test_tgamma_negsmall)(void) { return makemathname(tgamma)(-SMALL); }
FLOAT_T makemathname(test_hypot_big)(void) { return makemathname(hypot)(makemathname(big), makemathname(big)); }
FLOAT_T makemathname(test_lgamma_0)(void) { return makemathname(lgamma)(makemathname(zero)); }
FLOAT_T makemathname(test_lgamma_neg0)(void) { return makemathname(lgamma)(makemathname(negzero)); }
FLOAT_T makemathname(test_lgamma_neg1)(void) { return makemathname(lgamma)(-makemathname(one)); }
FLOAT_T makemathname(test_lgamma_big)(void) { return makemathname(lgamma)(makemathname(big)); }
FLOAT_T makemathname(test_lgamma_negbig)(void) { return makemathname(lgamma)(makemathname(-big)); }
FLOAT_T makemathname(test_lgamma_inf)(void) { return makemathname(lgamma)((FLOAT_T)INFINITY); }
FLOAT_T makemathname(test_lgamma_neginf)(void) { return makemathname(lgamma)(-(FLOAT_T)INFINITY); }
FLOAT_T makemathname(test_lgamma_r_0)(void) { return makemathname_r(lgamma)(makemathname(zero),&_signgam); }
FLOAT_T makemathname(test_lgamma_r_neg0)(void) { return makemathname_r(lgamma)(makemathname(negzero),&_signgam); }
FLOAT_T makemathname(test_lgamma_r_neg1)(void) { return makemathname_r(lgamma)(-makemathname(one),&_signgam); }
FLOAT_T makemathname(test_lgamma_r_big)(void) { return makemathname_r(lgamma)(makemathname(big),&_signgam); }
FLOAT_T makemathname(test_lgamma_r_negbig)(void) { return makemathname_r(lgamma)(makemathname(-big),&_signgam); }
FLOAT_T makemathname(test_lgamma_r_inf)(void) { return makemathname_r(lgamma)((FLOAT_T)INFINITY,&_signgam); }
FLOAT_T makemathname(test_lgamma_r_neginf)(void) { return makemathname_r(lgamma)(-(FLOAT_T)INFINITY,&_signgam); }
FLOAT_T makemathname(test_gamma_0)(void) { return makemathname(gamma)(makemathname(zero)); }
FLOAT_T makemathname(test_gamma_neg0)(void) { return makemathname(gamma)(makemathname(negzero)); }
FLOAT_T makemathname(test_gamma_neg1)(void) { return makemathname(gamma)(-makemathname(one)); }
FLOAT_T makemathname(test_gamma_big)(void) { return makemathname(gamma)(makemathname(big)); }
FLOAT_T makemathname(test_gamma_negbig)(void) { return makemathname(gamma)(makemathname(-big)); }
FLOAT_T makemathname(test_gamma_inf)(void) { return makemathname(gamma)((FLOAT_T)INFINITY); }
FLOAT_T makemathname(test_gamma_neginf)(void) { return makemathname(gamma)(-(FLOAT_T)INFINITY); }
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
FLOAT_T makemathname(test_scalbn_big)(void) { return makemathname(scalbn)(makemathname(one), 0x7fffffff); }
FLOAT_T makemathname(test_scalbn_tiny)(void) { return makemathname(scalbn)(makemathname(one), -0x7fffffff); }

#ifndef FE_DIVBYZERO
#define FE_DIVBYZERO 0
#endif
#ifndef FE_INEXACT
#define FE_INEXACT 0
#endif
#ifndef FE_INVALID
#define FE_INVALID 0
#endif
#ifndef FE_OVERFLOW
#define FE_OVERFLOW 0
#endif
#ifndef FE_UNDERFLOW
#define FE_UNDERFLOW 0
#endif

struct {
	FLOAT_T	(*func)(void);
	char	*name;
	FLOAT_T	value;
	int	except;
	int	errno_expect;
} makemathname(tests)[] = {
	TEST(acos_2, (FLOAT_T) NAN, FE_INVALID, EDOM),
	TEST(acosh_half, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(asin_2, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(exp_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(exp_neg, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),
	TEST(hypot_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(tgamma_0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(tgamma_neg0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(tgamma_neg1, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(tgamma_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(tgamma_negbig, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(tgamma_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(tgamma_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(tgamma_small, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(tgamma_negsmall, -(FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(lgamma_0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_neg0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_neg1, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(lgamma_negbig, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(lgamma_neginf, (FLOAT_T)INFINITY, 0, 0),
	TEST(lgamma_r_0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_neg0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_neg1, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(lgamma_r_negbig, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(lgamma_r_neginf, (FLOAT_T)INFINITY, 0, 0),
	TEST(gamma_0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(gamma_neg0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(gamma_neg1, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(gamma_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(gamma_negbig, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(gamma_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(gamma_neginf, (FLOAT_T)INFINITY, 0, 0),
	TEST(log_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(log10_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log10_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(pow_neg_real, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(pow_0_neg, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(pow_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow_tiny, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),
	TEST(remainder_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(sqrt_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(y0_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(y0_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(y1_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(y1_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(yn_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(yn_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(scalbn_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
        TEST(scalbn_tiny, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),
	{ 0 },
};

int
makemathname(run_tests)(void) {
	int result = 0;
	volatile FLOAT_T v;
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
		       makemathname(tests)[t].name, (double) v, err, strerror(err), except);
#else
		printf("    %-20.20s = (float) errno %d (%s) except 0x%x\n",
		       makemathname(tests)[t].name, err, strerror(err), except);
#endif
		if ((isinf(v) && isinf(makemathname(tests)[t].value) && ((v > 0) != (makemathname(tests)[t].value > 0))) ||
		    (v != makemathname(tests)[t].value && (!isnanf(v) || !isnanf(makemathname(tests)[t].value))))
		{
			printf("\tbad value got %g expect %g\n", (double) v, (double) makemathname(tests)[t].value);
			++result;
		}
		if (math_errhandling & EXCEPTION_TEST) {
			if ((except & makemathname(tests)[t].except) != makemathname(tests)[t].except) {
				printf("\texceptions supported. %s returns 0x%x instead of 0x%x\n",
                                       makemathname(tests)[t].name, except, makemathname(tests)[t].except);
				++result;
			}
		} else {
			if (except) {
				printf("\texceptions not supported. %s returns 0x%x\n", makemathname(tests)[t].name, except);
				++result;
			}
		}
		if (math_errhandling & MATH_ERRNO) {
			if (err != makemathname(tests)[t].errno_expect) {
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
