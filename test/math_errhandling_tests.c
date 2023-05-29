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

#define makelname(s) scat(s,l)

volatile FLOAT_T makemathname(zero) = (FLOAT_T) 0.0;
volatile FLOAT_T makemathname(negzero) = (FLOAT_T) -0.0;
volatile FLOAT_T makemathname(one) = (FLOAT_T) 1.0;
volatile FLOAT_T makemathname(two) = (FLOAT_T) 2.0;
volatile FLOAT_T makemathname(three) = (FLOAT_T) 3.0;
volatile FLOAT_T makemathname(half) = (FLOAT_T) 0.5;
volatile FLOAT_T makemathname(big) = BIG;
volatile FLOAT_T makemathname(bigodd) = BIGODD;
volatile FLOAT_T makemathname(bigeven) = BIGEVEN;
volatile FLOAT_T makemathname(small) = SMALL;
volatile FLOAT_T makemathname(infval) = (FLOAT_T) INFINITY;
volatile FLOAT_T makemathname(minfval) = (FLOAT_T) -INFINITY;
volatile FLOAT_T makemathname(qnanval) = (FLOAT_T) NAN;
volatile FLOAT_T makemathname(snanval) = (FLOAT_T) sNAN;
volatile FLOAT_T makemathname(pio2) = (FLOAT_T) (PI_VAL/(FLOAT_T)2.0);
volatile FLOAT_T makemathname(min_val) = (FLOAT_T) MIN_VAL;
volatile FLOAT_T makemathname(max_val) = (FLOAT_T) MAX_VAL;

FLOAT_T makemathname(scalb)(FLOAT_T, FLOAT_T);

#define cat2(a,b) a ## b
#define str(a) #a
#define TEST(n,v,ex,er)	{ .func = makemathname(cat2(test_, n)), .name = str(n), .value = (v), .except = (ex), .errno_expect = (er) }

static int _signgam;

#ifndef SIMPLE_MATH_ONLY
FLOAT_T makemathname(test_acos_2)(void) { return makemathname(acos)(makemathname(two)); }
FLOAT_T makemathname(test_acos_qnan)(void) { return makemathname(acos)(makemathname(qnanval)); }
FLOAT_T makemathname(test_acos_snan)(void) { return makemathname(acos)(makemathname(snanval)); }
FLOAT_T makemathname(test_acos_inf)(void) { return makemathname(acos)(makemathname(infval)); }
FLOAT_T makemathname(test_acos_minf)(void) { return makemathname(acos)(makemathname(minfval)); }

FLOAT_T makemathname(test_acosh_half)(void) { return makemathname(acosh)(makemathname(one)/makemathname(two)); }
FLOAT_T makemathname(test_acosh_qnan)(void) { return makemathname(acosh)(makemathname(qnanval)); }
FLOAT_T makemathname(test_acosh_snan)(void) { return makemathname(acosh)(makemathname(snanval)); }
FLOAT_T makemathname(test_acosh_inf)(void) { return makemathname(acosh)(makemathname(infval)); }
FLOAT_T makemathname(test_acosh_minf)(void) { return makemathname(acosh)(makemathname(minfval)); }

FLOAT_T makemathname(test_asin_2)(void) { return makemathname(asin)(makemathname(two)); }
FLOAT_T makemathname(test_asin_qnan)(void) { return makemathname(asin)(makemathname(qnanval)); }
FLOAT_T makemathname(test_asin_snan)(void) { return makemathname(asin)(makemathname(snanval)); }
FLOAT_T makemathname(test_asin_inf)(void) { return makemathname(asin)(makemathname(infval)); }
FLOAT_T makemathname(test_asin_minf)(void) { return makemathname(asin)(makemathname(minfval)); }

FLOAT_T makemathname(test_asinh_0)(void) { return makemathname(asinh)(makemathname(zero)); }
FLOAT_T makemathname(test_asinh_neg0)(void) { return makemathname(asinh)(makemathname(negzero)); }
FLOAT_T makemathname(test_asinh_qnan)(void) { return makemathname(asinh)(makemathname(qnanval)); }
FLOAT_T makemathname(test_asinh_snan)(void) { return makemathname(asinh)(makemathname(snanval)); }
FLOAT_T makemathname(test_asinh_inf)(void) { return makemathname(asinh)(makemathname(infval)); }
FLOAT_T makemathname(test_asinh_minf)(void) { return makemathname(asinh)(makemathname(minfval)); }

FLOAT_T makemathname(test_atan2_qnanx)(void) { return makemathname(atan2)(makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_atan2_qnany)(void) { return makemathname(atan2)(makemathname(qnanval), makemathname(one)); }
FLOAT_T makemathname(test_atan2_snanx)(void) { return makemathname(atan2)(makemathname(one), makemathname(snanval)); }
FLOAT_T makemathname(test_atan2_snany)(void) { return makemathname(atan2)(makemathname(snanval), makemathname(one)); }
FLOAT_T makemathname(test_atan2_tiny)(void) { return makemathname(atan2)(makemathname(min_val), makemathname(max_val)); }
FLOAT_T makemathname(test_atan2_nottiny)(void) { return makemathname(atan2)(makemathname(min_val), (FLOAT_T) -0x8p-20); }

FLOAT_T makemathname(test_atanh_qnan)(void) { return makemathname(atanh)(makemathname(qnanval)); }
FLOAT_T makemathname(test_atanh_snan)(void) { return makemathname(atanh)(makemathname(snanval)); }
FLOAT_T makemathname(test_atanh_1)(void) { return makemathname(atanh)(makemathname(one)); }
FLOAT_T makemathname(test_atanh_neg1)(void) { return makemathname(atanh)(-makemathname(one)); }
FLOAT_T makemathname(test_atanh_2)(void) { return makemathname(atanh)(makemathname(two)); }
FLOAT_T makemathname(test_atanh_neg2)(void) { return makemathname(atanh)(-makemathname(two)); }

FLOAT_T makemathname(test_cbrt_0)(void) { return makemathname(cbrt)(makemathname(zero)); }
FLOAT_T makemathname(test_cbrt_neg0)(void) { return makemathname(cbrt)(-makemathname(zero)); }
FLOAT_T makemathname(test_cbrt_inf)(void) { return makemathname(cbrt)(makemathname(infval)); }
FLOAT_T makemathname(test_cbrt_neginf)(void) { return makemathname(cbrt)(-makemathname(infval)); }
FLOAT_T makemathname(test_cbrt_qnan)(void) { return makemathname(cbrt)(makemathname(qnanval)); }
FLOAT_T makemathname(test_cbrt_snan)(void) { return makemathname(cbrt)(makemathname(snanval)); }

FLOAT_T makemathname(test_cos_inf)(void) { return makemathname(cos)(makemathname(infval)); }
FLOAT_T makemathname(test_cos_qnan)(void) { return makemathname(cos)(makemathname(qnanval)); }
FLOAT_T makemathname(test_cos_snan)(void) { return makemathname(cos)(makemathname(snanval)); }
FLOAT_T makemathname(test_cos_0)(void) { return makemathname(cos)(makemathname(zero)); }

FLOAT_T makemathname(test_cosh_big)(void) { return makemathname(cosh)(makemathname(big)); }
FLOAT_T makemathname(test_cosh_negbig)(void) { return makemathname(cosh)(makemathname(big)); }
FLOAT_T makemathname(test_cosh_inf)(void) { return makemathname(cosh)(makemathname(infval)); }
FLOAT_T makemathname(test_cosh_qnan)(void) { return makemathname(cosh)(makemathname(qnanval)); }
FLOAT_T makemathname(test_cosh_snan)(void) { return makemathname(cosh)(makemathname(snanval)); }

FLOAT_T makemathname(test_drem_0)(void) { return makemathname(drem)(makemathname(two), makemathname(zero)); }
FLOAT_T makemathname(test_drem_qnan_1)(void) { return makemathname(drem)(makemathname(qnanval), makemathname(one)); }
FLOAT_T makemathname(test_drem_1_qnan)(void) { return makemathname(drem)(makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_drem_snan_1)(void) { return makemathname(drem)(makemathname(snanval), makemathname(one)); }
FLOAT_T makemathname(test_drem_1_snan)(void) { return makemathname(drem)(makemathname(one), makemathname(snanval)); }
FLOAT_T makemathname(test_drem_inf_2)(void) { return makemathname(drem)(makemathname(infval), makemathname(two)); }
FLOAT_T makemathname(test_drem_inf_0)(void) { return makemathname(drem)(makemathname(infval), makemathname(zero)); }
FLOAT_T makemathname(test_drem_2_0)(void) { return makemathname(drem)(makemathname(two), makemathname(zero)); }
FLOAT_T makemathname(test_drem_1_2)(void) { return makemathname(drem)(makemathname(one), makemathname(two)); }
FLOAT_T makemathname(test_drem_neg1_2)(void) { return makemathname(drem)(-makemathname(one), makemathname(two)); }

FLOAT_T makemathname(test_erf_qnan)(void) { return makemathname(erf)(makemathname(qnanval)); }
FLOAT_T makemathname(test_erf_snan)(void) { return makemathname(erf)(makemathname(snanval)); }
FLOAT_T makemathname(test_erf_0)(void) { return makemathname(erf)(makemathname(zero)); }
FLOAT_T makemathname(test_erf_neg0)(void) { return makemathname(erf)(-makemathname(zero)); }
FLOAT_T makemathname(test_erf_inf)(void) { return makemathname(erf)(makemathname(infval)); }
FLOAT_T makemathname(test_erf_neginf)(void) { return makemathname(erf)(-makemathname(infval)); }
FLOAT_T makemathname(test_erf_small)(void) { return makemathname(erf)(makemathname(small)); }

FLOAT_T makemathname(test_exp_qnan)(void) { return makemathname(exp)(makemathname(qnanval)); }
FLOAT_T makemathname(test_exp_snan)(void) { return makemathname(exp)(makemathname(snanval)); }
FLOAT_T makemathname(test_exp_inf)(void) { return makemathname(exp)(makemathname(infval)); }
FLOAT_T makemathname(test_exp_neginf)(void) { return makemathname(exp)(-makemathname(infval)); }
FLOAT_T makemathname(test_exp_big)(void) { return makemathname(exp)(makemathname(big)); }
FLOAT_T makemathname(test_exp_negbig)(void) { return makemathname(exp)(-makemathname(big)); }

FLOAT_T makemathname(test_exp2_qnan)(void) { return makemathname(exp2)(makemathname(qnanval)); }
FLOAT_T makemathname(test_exp2_snan)(void) { return makemathname(exp2)(makemathname(snanval)); }
FLOAT_T makemathname(test_exp2_inf)(void) { return makemathname(exp2)(makemathname(infval)); }
FLOAT_T makemathname(test_exp2_neginf)(void) { return makemathname(exp2)(-makemathname(infval)); }
FLOAT_T makemathname(test_exp2_big)(void) { return makemathname(exp2)(makemathname(big)); }
FLOAT_T makemathname(test_exp2_negbig)(void) { return makemathname(exp2)(-makemathname(big)); }

FLOAT_T makemathname(test_exp10_qnan)(void) { return makemathname(exp10)(makemathname(qnanval)); }
FLOAT_T makemathname(test_exp10_snan)(void) { return makemathname(exp10)(makemathname(snanval)); }
FLOAT_T makemathname(test_exp10_inf)(void) { return makemathname(exp10)(makemathname(infval)); }
FLOAT_T makemathname(test_exp10_neginf)(void) { return makemathname(exp10)(-makemathname(infval)); }
FLOAT_T makemathname(test_exp10_big)(void) { return makemathname(exp10)(makemathname(big)); }
FLOAT_T makemathname(test_exp10_negbig)(void) { return makemathname(exp10)(-makemathname(big)); }

FLOAT_T makemathname(test_expm1_qnan)(void) { return makemathname(expm1)(makemathname(qnanval)); }
FLOAT_T makemathname(test_expm1_snan)(void) { return makemathname(expm1)(makemathname(snanval)); }
FLOAT_T makemathname(test_expm1_0)(void) { return makemathname(expm1)(makemathname(zero)); }
FLOAT_T makemathname(test_expm1_neg0)(void) { return makemathname(expm1)(-makemathname(zero)); }
FLOAT_T makemathname(test_expm1_inf)(void) { return makemathname(expm1)(makemathname(infval)); }
FLOAT_T makemathname(test_expm1_neginf)(void) { return makemathname(expm1)(-makemathname(infval)); }
FLOAT_T makemathname(test_expm1_big)(void) { return makemathname(expm1)(makemathname(big)); }
FLOAT_T makemathname(test_expm1_negbig)(void) { return makemathname(expm1)(-makemathname(big)); }

#endif /* SIMPLE_MATH_ONLY */

FLOAT_T makemathname(test_fabs_qnan)(void) { return makemathname(fabs)(makemathname(qnanval)); }
FLOAT_T makemathname(test_fabs_snan)(void) { return makemathname(fabs)(makemathname(snanval)); }
FLOAT_T makemathname(test_fabs_0)(void) { return makemathname(fabs)(makemathname(zero)); }
FLOAT_T makemathname(test_fabs_neg0)(void) { return makemathname(fabs)(-makemathname(zero)); }
FLOAT_T makemathname(test_fabs_inf)(void) { return makemathname(fabs)(makemathname(infval)); }
FLOAT_T makemathname(test_fabs_neginf)(void) { return makemathname(fabs)(-makemathname(infval)); }

#ifndef SIMPLE_MATH_ONLY

FLOAT_T makemathname(test_fdim_qnan_1)(void) { return makemathname(fdim)(makemathname(qnanval), makemathname(one)); }
FLOAT_T makemathname(test_fdim_1_qnan)(void) { return makemathname(fdim)(makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_fdim_snan_1)(void) { return makemathname(fdim)(makemathname(snanval), makemathname(one)); }
FLOAT_T makemathname(test_fdim_1_snan)(void) { return makemathname(fdim)(makemathname(one), makemathname(snanval)); }
FLOAT_T makemathname(test_fdim_inf_1)(void) { return makemathname(fdim)(makemathname(infval), makemathname(one)); }
FLOAT_T makemathname(test_fdim_1_inf)(void) { return makemathname(fdim)(makemathname(one), makemathname(infval)); }
FLOAT_T makemathname(test_fdim_big_negbig)(void) { return makemathname(fdim)(makemathname(big), -makemathname(big)); }

#endif

FLOAT_T makemathname(test_floor_1)(void) { return makemathname(floor)(makemathname(one)); }
FLOAT_T makemathname(test_floor_0)(void) { return makemathname(floor)(makemathname(zero)); }
FLOAT_T makemathname(test_floor_neg0)(void) { return makemathname(floor)(-makemathname(zero)); }
FLOAT_T makemathname(test_floor_qnan)(void) { return makemathname(floor)(makemathname(qnanval)); }
FLOAT_T makemathname(test_floor_snan)(void) { return makemathname(floor)(makemathname(snanval)); }
FLOAT_T makemathname(test_floor_inf)(void) { return makemathname(floor)(makemathname(infval)); }
FLOAT_T makemathname(test_floor_neginf)(void) { return makemathname(floor)(-makemathname(infval)); }

#ifndef SIMPLE_MATH_ONLY

FLOAT_T makemathname(test_fma_big_big_1)(void) { return makemathname(fma)(makemathname(big), makemathname(big), makemathname(one)); }
FLOAT_T makemathname(test_fma_big_negbig_1)(void) { return makemathname(fma)(makemathname(big), -makemathname(big), makemathname(one)); }
FLOAT_T makemathname(test_fma_small_small_1)(void) { return makemathname(fma)(makemathname(small), makemathname(small), makemathname(one)); }
FLOAT_T makemathname(test_fma_small_negsmall_1)(void) { return makemathname(fma)(makemathname(small), -makemathname(small), makemathname(one)); }
FLOAT_T makemathname(test_fma_small_small_0)(void) { return makemathname(fma)(makemathname(small), makemathname(small), makemathname(zero)); }
FLOAT_T makemathname(test_fma_small_negsmall_0)(void) { return makemathname(fma)(makemathname(small), -makemathname(small), makemathname(zero)); }
FLOAT_T makemathname(test_fma_qnan_1_1)(void) { return makemathname(fma)(makemathname(qnanval), makemathname(one), makemathname(one)); }
FLOAT_T makemathname(test_fma_1_qnan_1)(void) { return makemathname(fma)(makemathname(one), makemathname(qnanval), makemathname(one)); }
FLOAT_T makemathname(test_fma_1_1_qnan)(void) { return makemathname(fma)(makemathname(one), makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_fma_snan_1_1)(void) { return makemathname(fma)(makemathname(snanval), makemathname(one), makemathname(one)); }
FLOAT_T makemathname(test_fma_1_snan_1)(void) { return makemathname(fma)(makemathname(one), makemathname(snanval), makemathname(one)); }
FLOAT_T makemathname(test_fma_1_1_snan)(void) { return makemathname(fma)(makemathname(one), makemathname(one), makemathname(snanval)); }
FLOAT_T makemathname(test_fma_inf_1_neginf)(void) { return makemathname(fma)(makemathname(infval), makemathname(one), -makemathname(infval)); }
FLOAT_T makemathname(test_fma_1_inf_neginf)(void) { return makemathname(fma)(makemathname(one), makemathname(infval), -makemathname(infval)); }
FLOAT_T makemathname(test_fma_neginf_1_inf)(void) { return makemathname(fma)(makemathname(one), -makemathname(infval), makemathname(infval)); }
FLOAT_T makemathname(test_fma_1_neginf_inf)(void) { return makemathname(fma)(-makemathname(infval), makemathname(one), makemathname(infval)); }
FLOAT_T makemathname(test_fma_inf_0_1)(void) { return makemathname(fma)(makemathname(infval), makemathname(zero), makemathname(one)); }
FLOAT_T makemathname(test_fma_0_inf_1)(void) { return makemathname(fma)(makemathname(zero), makemathname(infval), makemathname(one)); }
FLOAT_T makemathname(test_fma_inf_0_qnan)(void) { return makemathname(fma)(makemathname(infval), makemathname(zero), makemathname(qnanval)); }
FLOAT_T makemathname(test_fma_0_inf_qnan)(void) { return makemathname(fma)(makemathname(zero), makemathname(infval), makemathname(qnanval)); }
FLOAT_T makemathname(test_fma_inf_0_snan)(void) { return makemathname(fma)(makemathname(infval), makemathname(zero), makemathname(snanval)); }
FLOAT_T makemathname(test_fma_0_inf_snan)(void) { return makemathname(fma)(makemathname(zero), makemathname(infval), makemathname(snanval)); }

#endif

FLOAT_T makemathname(test_fmax_qnan_qnan)(void) { return makemathname(fmax)(makemathname(qnanval), makemathname(qnanval)); }
FLOAT_T makemathname(test_fmax_qnan_1)(void) { return makemathname(fmax)(makemathname(qnanval), makemathname(one)); }
FLOAT_T makemathname(test_fmax_1_qnan)(void) { return makemathname(fmax)(makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_fmax_snan_snan)(void) { return makemathname(fmax)(makemathname(snanval), makemathname(snanval)); }
FLOAT_T makemathname(test_fmax_snan_1)(void) { return makemathname(fmax)(makemathname(snanval), makemathname(one)); }
FLOAT_T makemathname(test_fmax_1_snan)(void) { return makemathname(fmax)(makemathname(one), makemathname(snanval)); }

FLOAT_T makemathname(test_fmin_qnan_qnan)(void) { return makemathname(fmin)(makemathname(qnanval), makemathname(qnanval)); }
FLOAT_T makemathname(test_fmin_qnan_1)(void) { return makemathname(fmin)(makemathname(qnanval), makemathname(one)); }
FLOAT_T makemathname(test_fmin_1_qnan)(void) { return makemathname(fmin)(makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_fmin_snan_snan)(void) { return makemathname(fmin)(makemathname(snanval), makemathname(snanval)); }
FLOAT_T makemathname(test_fmin_snan_1)(void) { return makemathname(fmin)(makemathname(snanval), makemathname(one)); }
FLOAT_T makemathname(test_fmin_1_snan)(void) { return makemathname(fmin)(makemathname(one), makemathname(snanval)); }

#ifndef SIMPLE_MATH_ONLY

FLOAT_T makemathname(test_fmod_qnan_1)(void) { return makemathname(fmod)(makemathname(qnanval), makemathname(one)); }
FLOAT_T makemathname(test_fmod_1_qnan)(void) { return makemathname(fmod)(makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_fmod_snan_1)(void) { return makemathname(fmod)(makemathname(snanval), makemathname(one)); }
FLOAT_T makemathname(test_fmod_1_snan)(void) { return makemathname(fmod)(makemathname(one), makemathname(snanval)); }
FLOAT_T makemathname(test_fmod_inf_1)(void) { return makemathname(fmod)(makemathname(infval), makemathname(one)); }
FLOAT_T makemathname(test_fmod_neginf_1)(void) { return makemathname(fmod)(-makemathname(infval), makemathname(one)); }
FLOAT_T makemathname(test_fmod_1_0)(void) { return makemathname(fmod)(makemathname(one), makemathname(zero)); }
FLOAT_T makemathname(test_fmod_0_1)(void) { return makemathname(fmod)(makemathname(zero), makemathname(one)); }
FLOAT_T makemathname(test_fmod_neg0_1)(void) { return makemathname(fmod)(-makemathname(zero), makemathname(one)); }

#endif

FLOAT_T makemathname(test_hypot_big)(void) { return makemathname(hypot)(makemathname(big), makemathname(big)); }
FLOAT_T makemathname(test_hypot_1_qnan)(void) { return makemathname(hypot)(makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_hypot_qnan_1)(void) { return makemathname(hypot)(makemathname(qnanval), makemathname(one)); }
FLOAT_T makemathname(test_hypot_inf_qnan)(void) { return makemathname(hypot)(makemathname(infval), makemathname(qnanval)); }
FLOAT_T makemathname(test_hypot_neginf_qnan)(void) { return makemathname(hypot)(-makemathname(infval), makemathname(qnanval)); }
FLOAT_T makemathname(test_hypot_qnan_inf)(void) { return makemathname(hypot)(makemathname(qnanval), makemathname(infval)); }
FLOAT_T makemathname(test_hypot_qnan_neginf)(void) { return makemathname(hypot)(makemathname(qnanval), -makemathname(infval)); }
FLOAT_T makemathname(test_hypot_snan_inf)(void) { return makemathname(hypot)(makemathname(snanval), makemathname(infval)); }
FLOAT_T makemathname(test_hypot_snan_neginf)(void) { return makemathname(hypot)(makemathname(snanval), -makemathname(infval)); }
FLOAT_T makemathname(test_hypot_1_inf)(void) { return makemathname(hypot)(makemathname(one), makemathname(infval)); }
FLOAT_T makemathname(test_hypot_1_neginf)(void) { return makemathname(hypot)(makemathname(one), -makemathname(infval)); }
FLOAT_T makemathname(test_hypot_inf_1)(void) { return makemathname(hypot)(makemathname(infval), makemathname(one)); }
FLOAT_T makemathname(test_hypot_neginf_1)(void) { return makemathname(hypot)(-makemathname(infval), makemathname(one)); }

long makemathname(test_ilogb_0)(void) { return makemathname(ilogb)(makemathname(zero)); }
long makemathname(test_ilogb_qnan)(void) { return makemathname(ilogb)(makemathname(qnanval)); }
long makemathname(test_ilogb_snan)(void) { return makemathname(ilogb)(makemathname(snanval)); }
long makemathname(test_ilogb_inf)(void) { return makemathname(ilogb)(makemathname(infval)); }
long makemathname(test_ilogb_neginf)(void) { return makemathname(ilogb)(-makemathname(infval)); }

long makemathname(test_fpclassify_snan)(void) { return fpclassify(makemathname(snanval)); }
long makemathname(test_fpclassify_nan)(void) { return fpclassify(makemathname(qnanval)); }
long makemathname(test_fpclassify_inf)(void) { return fpclassify(makemathname(infval)); }
long makemathname(test_fpclassify_neginf)(void) { return fpclassify(-makemathname(infval)); }
long makemathname(test_fpclassify_zero)(void) { return fpclassify(makemathname(zero)); }
long makemathname(test_fpclassify_negzero)(void) { return fpclassify(-makemathname(zero)); }
long makemathname(test_fpclassify_small)(void) { return fpclassify(makemathname(small)); }
long makemathname(test_fpclassify_negsmall)(void) { return fpclassify(-makemathname(small)); }
long makemathname(test_fpclassify_two)(void) { return fpclassify(makemathname(two)); }
long makemathname(test_fpclassify_negtwo)(void) { return fpclassify(-makemathname(two)); }

long makemathname(test_isfinite_snan)(void) { return !!isfinite(makemathname(snanval)); }
long makemathname(test_isfinite_nan)(void) { return !!isfinite(makemathname(qnanval)); }
long makemathname(test_isfinite_inf)(void) { return !!isfinite(makemathname(infval)); }
long makemathname(test_isfinite_neginf)(void) { return !!isfinite(-makemathname(infval)); }
long makemathname(test_isfinite_zero)(void) { return !!isfinite(makemathname(zero)); }
long makemathname(test_isfinite_negzero)(void) { return !!isfinite(-makemathname(zero)); }
long makemathname(test_isfinite_small)(void) { return !!isfinite(makemathname(small)); }
long makemathname(test_isfinite_negsmall)(void) { return !!isfinite(-makemathname(small)); }
long makemathname(test_isfinite_two)(void) { return !!isfinite(makemathname(two)); }
long makemathname(test_isfinite_negtwo)(void) { return !!isfinite(-makemathname(two)); }

long makemathname(test_isnormal_snan)(void) { return !!isnormal(makemathname(snanval)); }
long makemathname(test_isnormal_nan)(void) { return !!isnormal(makemathname(qnanval)); }
long makemathname(test_isnormal_inf)(void) { return !!isnormal(makemathname(infval)); }
long makemathname(test_isnormal_neginf)(void) { return !!isnormal(-makemathname(infval)); }
long makemathname(test_isnormal_zero)(void) { return !!isnormal(makemathname(zero)); }
long makemathname(test_isnormal_negzero)(void) { return !!isnormal(-makemathname(zero)); }
long makemathname(test_isnormal_small)(void) { return !!isnormal(makemathname(small)); }
long makemathname(test_isnormal_negsmall)(void) { return !!isnormal(-makemathname(small)); }
long makemathname(test_isnormal_two)(void) { return !!isnormal(makemathname(two)); }
long makemathname(test_isnormal_negtwo)(void) { return !!isnormal(-makemathname(two)); }

long makemathname(test_isnan_snan)(void) { return !!isnan(makemathname(snanval)); }
long makemathname(test_isnan_nan)(void) { return !!isnan(makemathname(qnanval)); }
long makemathname(test_isnan_inf)(void) { return !!isnan(makemathname(infval)); }
long makemathname(test_isnan_neginf)(void) { return !!isnan(-makemathname(infval)); }
long makemathname(test_isnan_zero)(void) { return !!isnan(makemathname(zero)); }
long makemathname(test_isnan_negzero)(void) { return !!isnan(-makemathname(zero)); }
long makemathname(test_isnan_small)(void) { return !!isnan(makemathname(small)); }
long makemathname(test_isnan_negsmall)(void) { return !!isnan(-makemathname(small)); }
long makemathname(test_isnan_two)(void) { return !!isnan(makemathname(two)); }
long makemathname(test_isnan_negtwo)(void) { return !!isnan(-makemathname(two)); }

long makemathname(test_isinf_snan)(void) { return !!isinf(makemathname(snanval)); }
long makemathname(test_isinf_nan)(void) { return !!isinf(makemathname(qnanval)); }
long makemathname(test_isinf_inf)(void) { return !!isinf(makemathname(infval)); }
long makemathname(test_isinf_neginf)(void) { return !!isinf(-makemathname(infval)); }
long makemathname(test_isinf_zero)(void) { return !!isinf(makemathname(zero)); }
long makemathname(test_isinf_negzero)(void) { return !!isinf(-makemathname(zero)); }
long makemathname(test_isinf_small)(void) { return !!isinf(makemathname(small)); }
long makemathname(test_isinf_negsmall)(void) { return !!isinf(-makemathname(small)); }
long makemathname(test_isinf_two)(void) { return !!isinf(makemathname(two)); }
long makemathname(test_isinf_negtwo)(void) { return !!isinf(-makemathname(two)); }

#ifndef SIMPLE_MATH_ONLY

#ifndef NO_BESSEL_TESTS

FLOAT_T makemathname(test_j0_inf)(void) { return makemathname(j0)(makemathname(infval)); }
FLOAT_T makemathname(test_j0_qnan)(void) { return makemathname(j0)(makemathname(qnanval)); }
FLOAT_T makemathname(test_j0_snan)(void) { return makemathname(j0)(makemathname(snanval)); }

FLOAT_T makemathname(test_j1_inf)(void) { return makemathname(j1)(makemathname(infval)); }
FLOAT_T makemathname(test_j1_qnan)(void) { return makemathname(j1)(makemathname(qnanval)); }
FLOAT_T makemathname(test_j1_snan)(void) { return makemathname(j1)(makemathname(snanval)); }

FLOAT_T makemathname(test_jn_inf)(void) { return makemathname(jn)(3, makemathname(infval)); }
FLOAT_T makemathname(test_jn_qnan)(void) { return makemathname(jn)(3, makemathname(qnanval)); }
FLOAT_T makemathname(test_jn_snan)(void) { return makemathname(jn)(3, makemathname(snanval)); }
#endif

#endif

FLOAT_T makemathname(test_ldexp_1_0)(void) { return makemathname(ldexp)(makemathname(one), 0); }
FLOAT_T makemathname(test_ldexp_qnan_0)(void) { return makemathname(ldexp)(makemathname(qnanval), 0); }
FLOAT_T makemathname(test_ldexp_snan_0)(void) { return makemathname(ldexp)(makemathname(snanval), 0); }
FLOAT_T makemathname(test_ldexp_inf_0)(void) { return makemathname(ldexp)(makemathname(infval), 0); }
FLOAT_T makemathname(test_ldexp_neginf_0)(void) { return makemathname(ldexp)(-makemathname(infval), 0); }
FLOAT_T makemathname(test_ldexp_1_negbig)(void) { return makemathname(ldexp)(makemathname(one), -(__DBL_MAX_EXP__ * 100)); }
FLOAT_T makemathname(test_ldexp_1_big)(void) { return makemathname(ldexp)(makemathname(one),(__DBL_MAX_EXP__ * 100)); }

FLOAT_T makemathname(test_rint_qnan)(void) { return makemathname(rint)(makemathname(qnanval)); }
FLOAT_T makemathname(test_rint_snan)(void) { return makemathname(rint)(makemathname(snanval)); }
FLOAT_T makemathname(test_rint_inf)(void) { return makemathname(rint)(makemathname(infval)); }
FLOAT_T makemathname(test_rint_neginf)(void) { return makemathname(rint)(-makemathname(infval)); }
FLOAT_T makemathname(test_rint_big)(void) { return makemathname(rint)(makemathname(big)); }
FLOAT_T makemathname(test_rint_negbig)(void) { return makemathname(rint)(-makemathname(big)); }
FLOAT_T makemathname(test_rint_half)(void) { return makemathname(rint)(makemathname(half)); }
FLOAT_T makemathname(test_rint_neghalf)(void) { return makemathname(rint)(makemathname(half)); }

FLOAT_T makemathname(test_nearbyint_qnan)(void) { return makemathname(nearbyint)(makemathname(qnanval)); }
FLOAT_T makemathname(test_nearbyint_snan)(void) { return makemathname(nearbyint)(makemathname(snanval)); }
FLOAT_T makemathname(test_nearbyint_inf)(void) { return makemathname(nearbyint)(makemathname(infval)); }
FLOAT_T makemathname(test_nearbyint_neginf)(void) { return makemathname(nearbyint)(-makemathname(infval)); }
FLOAT_T makemathname(test_nearbyint_big)(void) { return makemathname(nearbyint)(makemathname(big)); }
FLOAT_T makemathname(test_nearbyint_negbig)(void) { return makemathname(nearbyint)(-makemathname(big)); }
FLOAT_T makemathname(test_nearbyint_half)(void) { return makemathname(nearbyint)(makemathname(half)); }
FLOAT_T makemathname(test_nearbyint_neghalf)(void) { return makemathname(nearbyint)(makemathname(half)); }

long makemathname(test_lrint_qnan)(void) { makemathname(lrint)(makemathname(qnanval)); return 0; }
long makemathname(test_lrint_snan)(void) { makemathname(lrint)(makemathname(snanval)); return 0; }
long makemathname(test_lrint_inf)(void) { makemathname(lrint)(makemathname(infval)); return 0; }
long makemathname(test_lrint_neginf)(void) { makemathname(lrint)(-makemathname(infval)); return 0; }
long makemathname(test_lrint_big)(void) { makemathname(lrint)(makemathname(big)); return 0; }
long makemathname(test_lrint_negbig)(void) { makemathname(lrint)(-makemathname(big)); return 0; }

long makemathname(test_lround_qnan)(void) { makemathname(lround)(makemathname(qnanval)); return 0; }
long makemathname(test_lround_snan)(void) { makemathname(lround)(makemathname(snanval)); return 0; }
long makemathname(test_lround_inf)(void) { makemathname(lround)(makemathname(infval)); return 0; }
long makemathname(test_lround_neginf)(void) { makemathname(lround)(-makemathname(infval)); return 0; }
long makemathname(test_lround_big)(void) { makemathname(lround)(makemathname(big)); return 0; }
long makemathname(test_lround_negbig)(void) { makemathname(lround)(-makemathname(big)); return 0; }

#ifndef SIMPLE_MATH_ONLY

FLOAT_T makemathname(test_nextafter_0_neg0)(void) { return makemathname(nextafter)(makemathname(zero), -makemathname(zero)); }
FLOAT_T makemathname(test_nextafter_neg0_0)(void) { return makemathname(nextafter)(-makemathname(zero), makemathname(zero)); }
FLOAT_T makemathname(test_nextafter_0_1)(void) { return makemathname(nextafter)(makemathname(zero), makemathname(one)); }
FLOAT_T makemathname(test_nextafter_0_neg1)(void) { return makemathname(nextafter)(makemathname(zero), -makemathname(one)); }
FLOAT_T makemathname(test_nextafter_min_1)(void) { return makemathname(nextafter)(makemathname(min_val), makemathname(one)); }
FLOAT_T makemathname(test_nextafter_negmin_neg1)(void) { return makemathname(nextafter)(-makemathname(min_val), -makemathname(one)); }
FLOAT_T makemathname(test_nextafter_qnan_1)(void) { return makemathname(nextafter)(makemathname(qnanval), makemathname(one)); }
FLOAT_T makemathname(test_nextafter_snan_1)(void) { return makemathname(nextafter)(makemathname(snanval), makemathname(one)); }
FLOAT_T makemathname(test_nextafter_1_qnan)(void) { return makemathname(nextafter)(makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_nextafter_1_snan)(void) { return makemathname(nextafter)(makemathname(one), makemathname(snanval)); }
FLOAT_T makemathname(test_nextafter_max_inf)(void) { return makemathname(nextafter)(makemathname(max_val), makemathname(infval)); }
FLOAT_T makemathname(test_nextafter_negmax_neginf)(void) { return makemathname(nextafter)(-makemathname(max_val), -makemathname(infval)); }
FLOAT_T makemathname(test_nextafter_min_0)(void) { return makemathname(nextafter)(makemathname(min_val), makemathname(zero)); }
FLOAT_T makemathname(test_nextafter_negmin_0)(void) { return makemathname(nextafter)(-makemathname(min_val), makemathname(zero)); }
FLOAT_T makemathname(test_nextafter_1_2)(void) {return makemathname(nextafter)(makemathname(one), makemathname(two)); }
FLOAT_T makemathname(test_nextafter_neg1_neg2)(void) {return makemathname(nextafter)(-makemathname(one), -makemathname(two)); }

#if defined(__SIZEOF_LONG_DOUBLE__) && !defined(NO_NEXTTOWARD)

FLOAT_T makemathname(test_nexttoward_0_neg0)(void) { return makemathname(nexttoward)(makemathname(zero), -makelname(zero)); }
FLOAT_T makemathname(test_nexttoward_neg0_0)(void) { return makemathname(nexttoward)(-makemathname(zero), makelname(zero)); }
FLOAT_T makemathname(test_nexttoward_0_1)(void) { return makemathname(nexttoward)(makemathname(zero), makelname(one)); }
FLOAT_T makemathname(test_nexttoward_0_neg1)(void) { return makemathname(nexttoward)(makemathname(zero), -makelname(one)); }
FLOAT_T makemathname(test_nexttoward_min_1)(void) { return makemathname(nexttoward)(makemathname(min_val), makelname(one)); }
FLOAT_T makemathname(test_nexttoward_negmin_neg1)(void) { return makemathname(nexttoward)(-makemathname(min_val), -makelname(one)); }
FLOAT_T makemathname(test_nexttoward_qnan_1)(void) { return makemathname(nexttoward)(makemathname(qnanval), makelname(one)); }
FLOAT_T makemathname(test_nexttoward_snan_1)(void) { return makemathname(nexttoward)(makemathname(snanval), makelname(one)); }
FLOAT_T makemathname(test_nexttoward_1_qnan)(void) { return makemathname(nexttoward)(makemathname(one), makelname(qnanval)); }
FLOAT_T makemathname(test_nexttoward_1_snan)(void) { return makemathname(nexttoward)(makemathname(one), makelname(snanval)); }
FLOAT_T makemathname(test_nexttoward_max_inf)(void) { return makemathname(nexttoward)(makemathname(max_val), makelname(infval)); }
FLOAT_T makemathname(test_nexttoward_negmax_neginf)(void) { return makemathname(nexttoward)(-makemathname(max_val), -makelname(infval)); }
FLOAT_T makemathname(test_nexttoward_min_0)(void) { return makemathname(nexttoward)(makemathname(min_val), makelname(zero)); }
FLOAT_T makemathname(test_nexttoward_negmin_0)(void) { return makemathname(nexttoward)(-makemathname(min_val), makelname(zero)); }
FLOAT_T makemathname(test_nexttoward_1_2)(void) {return makemathname(nexttoward)(makemathname(one), makelname(two)); }
FLOAT_T makemathname(test_nexttoward_neg1_neg2)(void) {return makemathname(nexttoward)(-makemathname(one), -makelname(two)); }

#endif

FLOAT_T makemathname(test_sin_inf)(void) { return makemathname(sin)(makemathname(infval)); }
FLOAT_T makemathname(test_sin_qnan)(void) { return makemathname(sin)(makemathname(qnanval)); }
FLOAT_T makemathname(test_sin_snan)(void) { return makemathname(sin)(makemathname(snanval)); }
FLOAT_T makemathname(test_sin_pio2)(void) { return makemathname(sin)(makemathname(pio2)); }
FLOAT_T makemathname(test_sin_small)(void) { return makemathname(sin)(makemathname(small)); }
FLOAT_T makemathname(test_sin_0)(void) { return makemathname(sin)(makemathname(zero)); }

/* This is mostly here to make sure sincos doesn't infinite loop due to compiler optimization */
FLOAT_T makemathname(test_sincos)(void) {
        FLOAT_T s, c;
        makemathname(sincos)(makemathname(one), &s, &c);
        FLOAT_T h = makemathname(sqrt)(s*s+c*c);
        return (FLOAT_T)0.9999 <= h && h <= (FLOAT_T)1.0001;
}
FLOAT_T makemathname(test_sincos_inf)(void) { FLOAT_T s, c; makemathname(sincos)(makemathname(infval), &s, &c); return s + c; }
FLOAT_T makemathname(test_sincos_qnan)(void) { FLOAT_T s, c; makemathname(sincos)(makemathname(qnanval), &s, &c); return s + c; }
FLOAT_T makemathname(test_sincos_snan)(void) { FLOAT_T s, c; makemathname(sincos)(makemathname(snanval), &s, &c); return s + c; }

FLOAT_T makemathname(test_sinh_qnan)(void) { return makemathname(sinh)(makemathname(qnanval)); }
FLOAT_T makemathname(test_sinh_snan)(void) { return makemathname(sinh)(makemathname(snanval)); }
FLOAT_T makemathname(test_sinh_0)(void) { return makemathname(sinh)(makemathname(zero)); }
FLOAT_T makemathname(test_sinh_neg0)(void) { return makemathname(sinh)(-makemathname(zero)); }
FLOAT_T makemathname(test_sinh_inf)(void) { return makemathname(sinh)(makemathname(infval)); }
FLOAT_T makemathname(test_sinh_neginf)(void) { return makemathname(sinh)(-makemathname(infval)); }

FLOAT_T makemathname(test_tgamma_qnan)(void) { return makemathname(tgamma)(makemathname(qnanval)); }
FLOAT_T makemathname(test_tgamma_snan)(void) { return makemathname(tgamma)(makemathname(snanval)); }
FLOAT_T makemathname(test_tgamma_0)(void) { return makemathname(tgamma)(makemathname(zero)); }
FLOAT_T makemathname(test_tgamma_neg0)(void) { return makemathname(tgamma)(makemathname(negzero)); }
FLOAT_T makemathname(test_tgamma_neg1)(void) { return makemathname(tgamma)(-makemathname(one)); }
FLOAT_T makemathname(test_tgamma_big)(void) { return makemathname(tgamma)(makemathname(big)); }
FLOAT_T makemathname(test_tgamma_negbig)(void) { return makemathname(tgamma)(makemathname(-big)); }
FLOAT_T makemathname(test_tgamma_inf)(void) { return makemathname(tgamma)(makemathname(infval)); }
FLOAT_T makemathname(test_tgamma_neginf)(void) { return makemathname(tgamma)(-makemathname(infval)); }
FLOAT_T makemathname(test_tgamma_small)(void) { return makemathname(tgamma)(makemathname(small)); }
FLOAT_T makemathname(test_tgamma_negsmall)(void) { return makemathname(tgamma)(-makemathname(small)); }

FLOAT_T makemathname(test_lgamma_qnan)(void) { return makemathname(lgamma)(makemathname(qnanval)); }
FLOAT_T makemathname(test_lgamma_snan)(void) { return makemathname(lgamma)(makemathname(snanval)); }
FLOAT_T makemathname(test_lgamma_1)(void) { return makemathname(lgamma)(makemathname(one)); }
FLOAT_T makemathname(test_lgamma_2)(void) { return makemathname(lgamma)(makemathname(two)); }
FLOAT_T makemathname(test_lgamma_inf)(void) { return makemathname(lgamma)(makemathname(infval)); }
FLOAT_T makemathname(test_lgamma_neginf)(void) { return makemathname(lgamma)(-makemathname(infval)); }
FLOAT_T makemathname(test_lgamma_0)(void) { return makemathname(lgamma)(makemathname(zero)); }
FLOAT_T makemathname(test_lgamma_neg0)(void) { return makemathname(lgamma)(makemathname(negzero)); }
FLOAT_T makemathname(test_lgamma_neg1)(void) { return makemathname(lgamma)(-makemathname(one)); }
FLOAT_T makemathname(test_lgamma_neg2)(void) { return makemathname(lgamma)(-makemathname(two)); }
FLOAT_T makemathname(test_lgamma_big)(void) { return makemathname(lgamma)(makemathname(big)); }
FLOAT_T makemathname(test_lgamma_negbig)(void) { return makemathname(lgamma)(makemathname(-big)); }

FLOAT_T makemathname(test_lgamma_r_qnan)(void) { return makemathname_r(lgamma)(makemathname(qnanval), &_signgam); }
FLOAT_T makemathname(test_lgamma_r_snan)(void) { return makemathname_r(lgamma)(makemathname(snanval), &_signgam); }
FLOAT_T makemathname(test_lgamma_r_1)(void) { return makemathname_r(lgamma)(makemathname(one), &_signgam); }
FLOAT_T makemathname(test_lgamma_r_2)(void) { return makemathname_r(lgamma)(makemathname(two), &_signgam); }
FLOAT_T makemathname(test_lgamma_r_inf)(void) { return makemathname_r(lgamma)(makemathname(infval), &_signgam); }
FLOAT_T makemathname(test_lgamma_r_neginf)(void) { return makemathname_r(lgamma)(-makemathname(infval), &_signgam); }
FLOAT_T makemathname(test_lgamma_r_0)(void) { return makemathname_r(lgamma)(makemathname(zero), &_signgam); }
FLOAT_T makemathname(test_lgamma_r_neg0)(void) { return makemathname_r(lgamma)(makemathname(negzero), &_signgam); }
FLOAT_T makemathname(test_lgamma_r_neg1)(void) { return makemathname_r(lgamma)(-makemathname(one), &_signgam); }
FLOAT_T makemathname(test_lgamma_r_neg2)(void) { return makemathname_r(lgamma)(-makemathname(two), &_signgam); }
FLOAT_T makemathname(test_lgamma_r_big)(void) { return makemathname_r(lgamma)(makemathname(big), &_signgam); }
FLOAT_T makemathname(test_lgamma_r_negbig)(void) { return makemathname_r(lgamma)(makemathname(-big), &_signgam); }

FLOAT_T makemathname(test_gamma_qnan)(void) { return makemathname(gamma)(makemathname(qnanval)); }
FLOAT_T makemathname(test_gamma_snan)(void) { return makemathname(gamma)(makemathname(snanval)); }
FLOAT_T makemathname(test_gamma_1)(void) { return makemathname(gamma)(makemathname(one)); }
FLOAT_T makemathname(test_gamma_2)(void) { return makemathname(gamma)(makemathname(two)); }
FLOAT_T makemathname(test_gamma_inf)(void) { return makemathname(gamma)(makemathname(infval)); }
FLOAT_T makemathname(test_gamma_neginf)(void) { return makemathname(gamma)(-makemathname(infval)); }
FLOAT_T makemathname(test_gamma_0)(void) { return makemathname(gamma)(makemathname(zero)); }
FLOAT_T makemathname(test_gamma_neg0)(void) { return makemathname(gamma)(makemathname(negzero)); }
FLOAT_T makemathname(test_gamma_neg1)(void) { return makemathname(gamma)(-makemathname(one)); }
FLOAT_T makemathname(test_gamma_neg2)(void) { return makemathname(gamma)(-makemathname(two)); }
FLOAT_T makemathname(test_gamma_big)(void) { return makemathname(gamma)(makemathname(big)); }
FLOAT_T makemathname(test_gamma_negbig)(void) { return makemathname(gamma)(makemathname(-big)); }

FLOAT_T makemathname(test_log_qnan)(void) { return makemathname(log)(makemathname(qnanval)); }
FLOAT_T makemathname(test_log_snan)(void) { return makemathname(log)(makemathname(snanval)); }
FLOAT_T makemathname(test_log_1)(void) { return makemathname(log)(makemathname(one)); }
FLOAT_T makemathname(test_log_inf)(void) { return makemathname(log)(makemathname(infval)); }
FLOAT_T makemathname(test_log_0)(void) { return makemathname(log)(makemathname(zero)); }
FLOAT_T makemathname(test_log_neg)(void) { return makemathname(log)(-makemathname(one)); }
FLOAT_T makemathname(test_log_neginf)(void) { return makemathname(log)(-makemathname(one)); }

FLOAT_T makemathname(test_log10_qnan)(void) { return makemathname(log10)(makemathname(qnanval)); }
FLOAT_T makemathname(test_log10_snan)(void) { return makemathname(log10)(makemathname(snanval)); }
FLOAT_T makemathname(test_log10_1)(void) { return makemathname(log10)(makemathname(one)); }
FLOAT_T makemathname(test_log10_inf)(void) { return makemathname(log10)(makemathname(infval)); }
FLOAT_T makemathname(test_log10_0)(void) { return makemathname(log10)(makemathname(zero)); }
FLOAT_T makemathname(test_log10_neg)(void) { return makemathname(log10)(-makemathname(one)); }
FLOAT_T makemathname(test_log10_neginf)(void) { return makemathname(log10)(-makemathname(one)); }

FLOAT_T makemathname(test_log1p_qnan)(void) { return makemathname(log1p)(makemathname(qnanval)); }
FLOAT_T makemathname(test_log1p_snan)(void) { return makemathname(log1p)(makemathname(snanval)); }
FLOAT_T makemathname(test_log1p_inf)(void) { return makemathname(log1p)(makemathname(infval)); }
FLOAT_T makemathname(test_log1p_neginf)(void) { return makemathname(log1p)(-makemathname(infval)); }
FLOAT_T makemathname(test_log1p_neg1)(void) { return makemathname(log1p)(-makemathname(one)); }
FLOAT_T makemathname(test_log1p_neg2)(void) { return makemathname(log1p)(-makemathname(two)); }

FLOAT_T makemathname(test_log2_qnan)(void) { return makemathname(log2)(makemathname(qnanval)); }
FLOAT_T makemathname(test_log2_snan)(void) { return makemathname(log2)(makemathname(snanval)); }
FLOAT_T makemathname(test_log2_1)(void) { return makemathname(log2)(makemathname(one)); }
FLOAT_T makemathname(test_log2_inf)(void) { return makemathname(log2)(makemathname(infval)); }
FLOAT_T makemathname(test_log2_0)(void) { return makemathname(log2)(makemathname(zero)); }
FLOAT_T makemathname(test_log2_neg)(void) { return makemathname(log2)(-makemathname(one)); }
FLOAT_T makemathname(test_log2_neginf)(void) { return makemathname(log2)(-makemathname(one)); }

#endif

FLOAT_T makemathname(test_logb_qnan)(void) { return makemathname(logb)(makemathname(qnanval)); }
FLOAT_T makemathname(test_logb_snan)(void) { return makemathname(logb)(makemathname(snanval)); }
FLOAT_T makemathname(test_logb_0)(void) { return makemathname(logb)(makemathname(zero)); }
FLOAT_T makemathname(test_logb_neg0)(void) { return makemathname(logb)(-makemathname(zero)); }
FLOAT_T makemathname(test_logb_inf)(void) { return makemathname(logb)(makemathname(infval)); }
FLOAT_T makemathname(test_logb_neginf)(void) { return makemathname(logb)(-makemathname(infval)); }

#ifndef SIMPLE_MATH_ONLY

FLOAT_T makemathname(test_pow_neg_half)(void) { return makemathname(pow)(-makemathname(two), makemathname(half)); }
FLOAT_T makemathname(test_pow_big)(void) { return makemathname(pow)(makemathname(two), makemathname(big)); }
FLOAT_T makemathname(test_pow_negbig)(void) { return makemathname(pow)(-makemathname(big), makemathname(three)); }
FLOAT_T makemathname(test_pow_tiny)(void) { return makemathname(pow)(makemathname(two), -makemathname(big)); }
FLOAT_T makemathname(test_pow_1_1)(void) { return makemathname(pow)(makemathname(one), makemathname(one)); }
FLOAT_T makemathname(test_pow_1_2)(void) { return makemathname(pow)(makemathname(one), makemathname(two)); }
FLOAT_T makemathname(test_pow_1_neg2)(void) { return makemathname(pow)(makemathname(one), makemathname(two)); }
FLOAT_T makemathname(test_pow_1_inf)(void) { return makemathname(pow)(makemathname(one), makemathname(infval)); }
FLOAT_T makemathname(test_pow_1_neginf)(void) { return makemathname(pow)(makemathname(one), -makemathname(infval)); }
FLOAT_T makemathname(test_pow_1_qnan)(void) { return makemathname(pow)(makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_pow_qnan_0)(void) { return makemathname(pow)(makemathname(qnanval), makemathname(zero)); }
FLOAT_T makemathname(test_pow_qnan_neg0)(void) { return makemathname(pow)(makemathname(qnanval), -makemathname(zero)); }
FLOAT_T makemathname(test_pow_1_snan)(void) { return makemathname(pow)(makemathname(one), makemathname(snanval)); }
FLOAT_T makemathname(test_pow_snan_0)(void) { return makemathname(pow)(makemathname(snanval), makemathname(zero)); }
FLOAT_T makemathname(test_pow_snan_neg0)(void) { return makemathname(pow)(makemathname(snanval), -makemathname(zero)); }
FLOAT_T makemathname(test_pow_1_0)(void) { return makemathname(pow)(makemathname(one), makemathname(zero)); }
FLOAT_T makemathname(test_pow_1_neg0)(void) { return makemathname(pow)(makemathname(one), -makemathname(zero)); }
FLOAT_T makemathname(test_pow_0_0)(void) { return makemathname(pow)(makemathname(zero), makemathname(zero)); }
FLOAT_T makemathname(test_pow_0_neg0)(void) { return makemathname(pow)(makemathname(zero), -makemathname(zero)); }
FLOAT_T makemathname(test_pow_inf_0)(void) { return makemathname(pow)(makemathname(infval), makemathname(zero)); }
FLOAT_T makemathname(test_pow_inf_neg0)(void) { return makemathname(pow)(makemathname(infval), -makemathname(zero)); }
FLOAT_T makemathname(test_pow_0_odd)(void) { return makemathname(pow)(makemathname(zero), makemathname(three)); }
FLOAT_T makemathname(test_pow_neg0_odd)(void) { return makemathname(pow)(-makemathname(zero), makemathname(three)); }
FLOAT_T makemathname(test_pow_neg1_inf)(void) { return makemathname(pow)(-makemathname(one), makemathname(infval)); }
FLOAT_T makemathname(test_pow_neg1_neginf)(void) { return makemathname(pow)(-makemathname(one), -makemathname(infval)); }
FLOAT_T makemathname(test_pow_half_neginf)(void) { return makemathname(pow)(makemathname(half), -makemathname(infval)); }
FLOAT_T makemathname(test_pow_neghalf_neginf)(void) { return makemathname(pow)(-makemathname(half), -makemathname(infval)); }
FLOAT_T makemathname(test_pow_2_neginf)(void) { return makemathname(pow)(makemathname(two), -makemathname(infval)); }
FLOAT_T makemathname(test_pow_neg2_neginf)(void) { return makemathname(pow)(-makemathname(two), -makemathname(infval)); }
FLOAT_T makemathname(test_pow_half_inf)(void) { return makemathname(pow)(makemathname(half), makemathname(infval)); }
FLOAT_T makemathname(test_pow_neghalf_inf)(void) { return makemathname(pow)(-makemathname(half), makemathname(infval)); }
FLOAT_T makemathname(test_pow_2_inf)(void) { return makemathname(pow)(makemathname(two), makemathname(infval)); }
FLOAT_T makemathname(test_pow_2_qnan)(void) { return makemathname(pow)(makemathname(two), makemathname(qnanval)); }
FLOAT_T makemathname(test_pow_2_snan)(void) { return makemathname(pow)(makemathname(two), makemathname(snanval)); }
FLOAT_T makemathname(test_pow_neg2_inf)(void) { return makemathname(pow)(-makemathname(two), makemathname(infval)); }
FLOAT_T makemathname(test_pow_neginf_negodd)(void) { return makemathname(pow)(-makemathname(infval), -makemathname(three)); }
FLOAT_T makemathname(test_pow_neginf_neghalf)(void) { return makemathname(pow)(-makemathname(infval), -makemathname(half)); }
FLOAT_T makemathname(test_pow_neginf_neg2)(void) { return makemathname(pow)(-makemathname(infval), -makemathname(two)); }
FLOAT_T makemathname(test_pow_neginf_odd)(void) { return makemathname(pow)(-makemathname(infval), makemathname(three)); }
FLOAT_T makemathname(test_pow_neginf_half)(void) { return makemathname(pow)(-makemathname(infval), makemathname(half)); }
FLOAT_T makemathname(test_pow_neginf_2)(void) { return makemathname(pow)(-makemathname(infval), makemathname(two)); }
FLOAT_T makemathname(test_pow_inf_neg2)(void) { return makemathname(pow)(makemathname(infval), -makemathname(two)); }
FLOAT_T makemathname(test_pow_inf_2)(void) { return makemathname(pow)(makemathname(infval), makemathname(two)); }
FLOAT_T makemathname(test_pow_0_neg2)(void) { return makemathname(pow)(makemathname(zero), -makemathname(two)); }
FLOAT_T makemathname(test_pow_neg0_neg2)(void) { return makemathname(pow)(-makemathname(zero), -makemathname(two)); }
FLOAT_T makemathname(test_pow_0_neghalf)(void) { return makemathname(pow)(makemathname(zero), -makemathname(half)); }
FLOAT_T makemathname(test_pow_neg0_neghalf)(void) { return makemathname(pow)(-makemathname(zero), -makemathname(half)); }
FLOAT_T makemathname(test_pow_0_neg3)(void) { return makemathname(pow)(makemathname(zero), -makemathname(three)); }
FLOAT_T makemathname(test_pow_neg0_neg3)(void) { return makemathname(pow)(-makemathname(zero), -makemathname(three)); }
FLOAT_T makemathname(test_pow_negsmall_negbigodd)(void) { return makemathname(pow)(-makemathname(small), -makemathname(bigodd)); }
FLOAT_T makemathname(test_pow_negbig_bigodd)(void) { return makemathname(pow)(-makemathname(big), makemathname(bigodd)); }
FLOAT_T makemathname(test_pow_negsmall_negbigeven)(void) { return makemathname(pow)(-makemathname(small), -makemathname(bigeven)); }
FLOAT_T makemathname(test_pow_negbig_bigeven)(void) { return makemathname(pow)(-makemathname(big), makemathname(bigeven)); }

#ifndef __PICOLIBC__
#define pow10(x) exp10(x)
#define pow10f(x) exp10f(x)
#define pow10l(x) exp10l(x)
#endif

FLOAT_T makemathname(test_pow10_qnan)(void) { return makemathname(pow10)(makemathname(qnanval)); }
FLOAT_T makemathname(test_pow10_snan)(void) { return makemathname(pow10)(makemathname(snanval)); }
FLOAT_T makemathname(test_pow10_inf)(void) { return makemathname(pow10)(makemathname(infval)); }
FLOAT_T makemathname(test_pow10_neginf)(void) { return makemathname(pow10)(-makemathname(infval)); }
FLOAT_T makemathname(test_pow10_big)(void) { return makemathname(pow10)(makemathname(big)); }
FLOAT_T makemathname(test_pow10_negbig)(void) { return makemathname(pow10)(-makemathname(big)); }

FLOAT_T makemathname(test_remainder_0)(void) { return makemathname(remainder)(makemathname(two), makemathname(zero)); }
FLOAT_T makemathname(test_remainder_qnan_1)(void) { return makemathname(remainder)(makemathname(qnanval), makemathname(one)); }
FLOAT_T makemathname(test_remainder_1_qnan)(void) { return makemathname(remainder)(makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_remainder_snan_1)(void) { return makemathname(remainder)(makemathname(snanval), makemathname(one)); }
FLOAT_T makemathname(test_remainder_1_snan)(void) { return makemathname(remainder)(makemathname(one), makemathname(snanval)); }
FLOAT_T makemathname(test_remainder_inf_2)(void) { return makemathname(remainder)(makemathname(infval), makemathname(two)); }
FLOAT_T makemathname(test_remainder_inf_0)(void) { return makemathname(remainder)(makemathname(infval), makemathname(zero)); }
FLOAT_T makemathname(test_remainder_2_0)(void) { return makemathname(remainder)(makemathname(two), makemathname(zero)); }
FLOAT_T makemathname(test_remainder_1_2)(void) { return makemathname(remainder)(makemathname(one), makemathname(two)); }
FLOAT_T makemathname(test_remainder_neg1_2)(void) { return makemathname(remainder)(-makemathname(one), makemathname(two)); }

#endif

FLOAT_T makemathname(test_sqrt_qnan)(void) { return makemathname(sqrt)(makemathname(qnanval)); }
FLOAT_T makemathname(test_sqrt_snan)(void) { return makemathname(sqrt)(makemathname(snanval)); }
FLOAT_T makemathname(test_sqrt_0)(void) { return makemathname(sqrt)(makemathname(zero)); }
FLOAT_T makemathname(test_sqrt_neg0)(void) { return makemathname(sqrt)(-makemathname(zero)); }
FLOAT_T makemathname(test_sqrt_inf)(void) { return makemathname(sqrt)(makemathname(infval)); }
FLOAT_T makemathname(test_sqrt_neginf)(void) { return makemathname(sqrt)(-makemathname(infval)); }
FLOAT_T makemathname(test_sqrt_neg)(void) { return makemathname(sqrt)(-makemathname(two)); }
FLOAT_T makemathname(test_sqrt_2)(void) { return makemathname(sqrt)(makemathname(two)); }

#ifndef SIMPLE_MATH_ONLY

FLOAT_T makemathname(test_tan_qnan)(void) { return makemathname(tan)(makemathname(qnanval)); }
FLOAT_T makemathname(test_tan_snan)(void) { return makemathname(tan)(makemathname(snanval)); }
FLOAT_T makemathname(test_tan_inf)(void) { return makemathname(tan)(makemathname(infval)); }
FLOAT_T makemathname(test_tan_neginf)(void) { return makemathname(tan)(-makemathname(infval)); }
FLOAT_T makemathname(test_tan_pio2)(void) { return makemathname(tan)(makemathname(pio2)); }

FLOAT_T makemathname(test_tanh_qnan)(void) { return makemathname(tanh)(makemathname(qnanval)); }
FLOAT_T makemathname(test_tanh_snan)(void) { return makemathname(tanh)(makemathname(snanval)); }
FLOAT_T makemathname(test_tanh_0)(void) { return makemathname(tanh)(makemathname(zero)); }
FLOAT_T makemathname(test_tanh_neg0)(void) { return makemathname(tanh)(-makemathname(zero)); }
FLOAT_T makemathname(test_tanh_inf)(void) { return makemathname(tanh)(makemathname(infval)); }
FLOAT_T makemathname(test_tanh_neginf)(void) { return makemathname(tanh)(-makemathname(infval)); }

#ifndef NO_BESSEL_TESTS
FLOAT_T makemathname(test_y0_qnan)(void) { return makemathname(y0)(makemathname(qnanval)); }
FLOAT_T makemathname(test_y0_snan)(void) { return makemathname(y0)(makemathname(snanval)); }
FLOAT_T makemathname(test_y0_inf)(void) { return makemathname(y0)(makemathname(infval)); }
FLOAT_T makemathname(test_y0_neg)(void) { return makemathname(y0)(-makemathname(one)); }
FLOAT_T makemathname(test_y0_0)(void) { return makemathname(y0)(makemathname(zero)); }

FLOAT_T makemathname(test_y1_qnan)(void) { return makemathname(y1)(makemathname(qnanval)); }
FLOAT_T makemathname(test_y1_snan)(void) { return makemathname(y1)(makemathname(snanval)); }
FLOAT_T makemathname(test_y1_inf)(void) { return makemathname(y1)(makemathname(infval)); }
FLOAT_T makemathname(test_y1_neg)(void) { return makemathname(y1)(-makemathname(one)); }
FLOAT_T makemathname(test_y1_0)(void) { return makemathname(y1)(makemathname(zero)); }

FLOAT_T makemathname(test_yn_qnan)(void) { return makemathname(yn)(2, makemathname(qnanval)); }
FLOAT_T makemathname(test_yn_snan)(void) { return makemathname(yn)(2, makemathname(snanval)); }
FLOAT_T makemathname(test_yn_inf)(void) { return makemathname(yn)(2, makemathname(infval)); }
FLOAT_T makemathname(test_yn_neg)(void) { return makemathname(yn)(2, -makemathname(one)); }
FLOAT_T makemathname(test_yn_0)(void) { return makemathname(yn)(2, makemathname(zero)); }
#endif

#endif

FLOAT_T makemathname(test_scalb_1_1)(void) { return makemathname(scalb)(makemathname(one), makemathname(one)); }
FLOAT_T makemathname(test_scalb_1_half)(void) { return makemathname(scalb)(makemathname(one), makemathname(half)); }
FLOAT_T makemathname(test_scalb_qnan_1)(void) { return makemathname(scalb)(makemathname(qnanval), makemathname(one)); }
FLOAT_T makemathname(test_scalb_1_qnan)(void) { return makemathname(scalb)(makemathname(one), makemathname(qnanval)); }
FLOAT_T makemathname(test_scalb_snan_1)(void) { return makemathname(scalb)(makemathname(snanval), makemathname(one)); }
FLOAT_T makemathname(test_scalb_1_snan)(void) { return makemathname(scalb)(makemathname(one), makemathname(snanval)); }
FLOAT_T makemathname(test_scalb_inf_2)(void) { return makemathname(scalb)(makemathname(infval), makemathname(two)); }
FLOAT_T makemathname(test_scalb_neginf_2)(void) { return makemathname(scalb)(-makemathname(infval), makemathname(two)); }
FLOAT_T makemathname(test_scalb_0_2)(void) { return makemathname(scalb)(makemathname(zero), makemathname(two)); }
FLOAT_T makemathname(test_scalb_neg0_2)(void) { return makemathname(scalb)(-makemathname(zero), makemathname(two)); }
FLOAT_T makemathname(test_scalb_0_inf)(void) { return makemathname(scalb)(makemathname(zero), makemathname(infval)); }
FLOAT_T makemathname(test_scalb_inf_neginf)(void) { return makemathname(scalb)(makemathname(infval), -makemathname(infval)); }
FLOAT_T makemathname(test_scalb_neginf_neginf)(void) { return makemathname(scalb)(-makemathname(infval), -makemathname(infval)); }
FLOAT_T makemathname(test_scalb_1_big)(void) { return makemathname(scalb)(makemathname(one), makemathname(big)); }
FLOAT_T makemathname(test_scalb_neg1_big)(void) { return makemathname(scalb)(-makemathname(one), makemathname(big)); }
FLOAT_T makemathname(test_scalb_1_negbig)(void) { return makemathname(scalb)(makemathname(one), -makemathname(big)); }
FLOAT_T makemathname(test_scalb_neg1_negbig)(void) { return makemathname(scalb)(-makemathname(one), -makemathname(big)); }

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

#define MY_EXCEPT (FE_DIVBYZERO|FE_INEXACT|FE_INVALID|FE_OVERFLOW|FE_UNDERFLOW)

#undef sNAN_RET
#undef sNAN_EXCEPTION
#if defined(__i386__) && !defined(TEST_LONG_DOUBLE)
/*
 * i386 ABI returns floats in the 8087 registers, which convert sNAN
 * to NAN on load, so you can't ever return a sNAN value successfully.
 */
#define sNAN_RET        NAN
#define sNAN_EXCEPTION  FE_INVALID
#else
#define sNAN_RET        sNAN
#define sNAN_EXCEPTION  0
#endif

struct {
	FLOAT_T	(*func)(void);
	char	*name;
	FLOAT_T	value;
	int	except;
	int	errno_expect;
} makemathname(tests)[] = {
#ifndef SIMPLE_MATH_ONLY
	TEST(acos_2, (FLOAT_T) NAN, FE_INVALID, EDOM),
	TEST(acos_qnan, (FLOAT_T) NAN, 0, 0),
	TEST(acos_snan, (FLOAT_T) NAN, FE_INVALID, 0),
	TEST(acos_inf, (FLOAT_T) NAN, FE_INVALID, EDOM),
	TEST(acos_minf, (FLOAT_T) NAN, FE_INVALID, EDOM),

	TEST(acosh_half, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(acosh_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(acosh_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(acosh_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(acosh_minf, (FLOAT_T)NAN, FE_INVALID, EDOM),

	TEST(asin_2, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(asin_qnan, (FLOAT_T) NAN, 0, 0),
	TEST(asin_snan, (FLOAT_T) NAN, FE_INVALID, 0),
	TEST(asin_inf, (FLOAT_T) NAN, FE_INVALID, EDOM),
	TEST(asin_minf, (FLOAT_T) NAN, FE_INVALID, EDOM),

        TEST(asinh_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(asinh_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(asinh_0, (FLOAT_T)0.0, 0, 0),
        TEST(asinh_neg0, (FLOAT_T)-0.0, 0, 0),
        TEST(asinh_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(asinh_minf, (FLOAT_T)-INFINITY, 0, 0),

        TEST(atan2_qnanx, (FLOAT_T)NAN, 0, 0),
        TEST(atan2_qnany, (FLOAT_T)NAN, 0, 0),
        TEST(atan2_snanx, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(atan2_snany, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(atan2_tiny, (FLOAT_T)0.0, FE_UNDERFLOW|FE_INEXACT, ERANGE),
        TEST(atan2_nottiny, PI_VAL, 0, 0),

        TEST(atanh_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(atanh_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(atanh_1, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
        TEST(atanh_neg1, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
        TEST(atanh_2, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(atanh_neg2, (FLOAT_T)NAN, FE_INVALID, EDOM),

        TEST(cbrt_0, (FLOAT_T)0.0, 0, 0),
        TEST(cbrt_neg0, -(FLOAT_T)0.0, 0, 0),
        TEST(cbrt_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(cbrt_neginf, -(FLOAT_T)INFINITY, 0, 0),
        TEST(cbrt_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(cbrt_snan, (FLOAT_T)NAN, FE_INVALID, 0),

        TEST(cos_inf, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(cos_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(cos_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(cos_0, (FLOAT_T)1.0, 0, 0),

        TEST(cosh_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(cosh_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(cosh_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(cosh_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
        TEST(cosh_negbig, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),

	TEST(drem_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(drem_qnan_1, (FLOAT_T)NAN, 0, 0),
	TEST(drem_1_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(drem_snan_1, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(drem_1_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(drem_inf_2, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(drem_inf_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(drem_2_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(drem_1_2, (FLOAT_T)1.0, 0, 0),
        TEST(drem_neg1_2, -(FLOAT_T)1.0, 0, 0),

        TEST(erf_qnan, (FLOAT_T) NAN, 0, 0),
        TEST(erf_snan, (FLOAT_T) NAN, FE_INVALID, 0),
        TEST(erf_0, (FLOAT_T) 0, 0, 0),
        TEST(erf_neg0, -(FLOAT_T) 0, 0, 0),
        TEST(erf_inf, (FLOAT_T) 1.0, 0, 0),
        TEST(erf_neginf, -(FLOAT_T) 1.0, 0, 0),
        TEST(erf_small, (FLOAT_T) 2.0 * (FLOAT_T) SMALL / (FLOAT_T) 1.772453850905516027298167, FE_UNDERFLOW, 0),

	TEST(exp_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(exp_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(exp_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(exp_neginf, (FLOAT_T)0.0, 0, 0),
	TEST(exp_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(exp_negbig, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),

	TEST(exp2_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(exp2_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(exp2_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(exp2_neginf, (FLOAT_T)0.0, 0, 0),
	TEST(exp2_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(exp2_negbig, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),

	TEST(exp10_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(exp10_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(exp10_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(exp10_neginf, (FLOAT_T)0.0, 0, 0),
	TEST(exp10_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(exp10_negbig, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),

	TEST(expm1_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(expm1_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(expm1_0, (FLOAT_T)0.0, 0, 0),
	TEST(expm1_neg0, -(FLOAT_T)0.0, 0, 0),
	TEST(expm1_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(expm1_neginf, -(FLOAT_T)1.0, 0, 0),
	TEST(expm1_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
#if !defined(__PICOLIBC__) && defined(TEST_LONG_DOUBLE) && (defined(__x86_64) || defined(__i386))
        /* glibc returns incorrect value on x86 */
	TEST(expm1_negbig, -(FLOAT_T)1.0, 0, 0),
#else
	TEST(expm1_negbig, -(FLOAT_T)1.0, FE_INEXACT, 0),
#endif

#endif
        TEST(fabs_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(fabs_snan, (FLOAT_T)sNAN_RET, sNAN_EXCEPTION, 0),
        TEST(fabs_0, (FLOAT_T)0.0, 0, 0),
        TEST(fabs_neg0, (FLOAT_T)0.0, 0, 0),
        TEST(fabs_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(fabs_neginf, (FLOAT_T)INFINITY, 0, 0),

#ifndef SIMPLE_MATH_ONLY
        TEST(fdim_qnan_1, (FLOAT_T)NAN, 0, 0),
        TEST(fdim_1_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(fdim_snan_1, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fdim_1_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fdim_inf_1, (FLOAT_T)INFINITY, 0, 0),
        TEST(fdim_1_inf, (FLOAT_T)0.0, 0, 0),
        TEST(fdim_big_negbig, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
#endif

        TEST(floor_1, (FLOAT_T)1.0, 0, 0),
        TEST(floor_0, (FLOAT_T)0.0, 0, 0),
        TEST(floor_neg0, -(FLOAT_T)0.0, 0, 0),
        TEST(floor_qnan, (FLOAT_T)NAN, 0, 0),
#ifdef __PICOLIBC__
        /* looks like glibc gets this wrong */
        TEST(floor_snan, (FLOAT_T)NAN, FE_INVALID, 0),
#endif
        TEST(floor_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(floor_neginf, -(FLOAT_T)INFINITY, 0, 0),

#ifndef SIMPLE_MATH_ONLY
        TEST(fma_big_big_1, (FLOAT_T)INFINITY, FE_OVERFLOW, 0),
        TEST(fma_big_negbig_1, -(FLOAT_T)INFINITY, FE_OVERFLOW, 0),
        TEST(fma_small_small_1, (FLOAT_T)1.0, FE_INEXACT, 0),
        TEST(fma_small_negsmall_1, (FLOAT_T)1.0, FE_INEXACT, 0),
        TEST(fma_small_small_0, (FLOAT_T)0.0, FE_UNDERFLOW, 0),
        TEST(fma_small_negsmall_0, -(FLOAT_T)0.0, FE_UNDERFLOW, 0),
        TEST(fma_qnan_1_1, (FLOAT_T)NAN, 0, 0),
        TEST(fma_1_qnan_1, (FLOAT_T)NAN, 0, 0),
        TEST(fma_1_1_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(fma_inf_1_neginf, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_1_inf_neginf, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_neginf_1_inf, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_1_neginf_inf, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_inf_0_1, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_0_inf_1, (FLOAT_T)NAN, FE_INVALID, 0),
#ifdef __PICOLIBC__
        /* Linux says these will set FE_INVALID, POSIX says optional, glibc does not set exception */
        TEST(fma_inf_0_qnan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_0_inf_qnan, (FLOAT_T)NAN, FE_INVALID, 0),
#endif
#endif

        TEST(fmax_qnan_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(fmax_qnan_1, (FLOAT_T)1.0, 0, 0),
        TEST(fmax_1_qnan, (FLOAT_T)1.0, 0, 0),

        TEST(fmin_qnan_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(fmin_qnan_1, (FLOAT_T)1.0, 0, 0),
        TEST(fmin_1_qnan, (FLOAT_T)1.0, 0, 0),

#ifndef SIMPLE_MATH_ONLY

        TEST(fmod_qnan_1, (FLOAT_T)NAN, 0, 0),
        TEST(fmod_1_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(fmod_inf_1, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(fmod_neginf_1, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(fmod_1_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(fmod_0_1, (FLOAT_T)0.0, 0, 0),
        TEST(fmod_neg0_1, -(FLOAT_T)0.0, 0, 0),

        TEST(gamma_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(gamma_1, (FLOAT_T) 0.0, 0, 0),
        TEST(gamma_2, (FLOAT_T) 0.0, 0, 0),
	TEST(gamma_0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(gamma_neg0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(gamma_neg1, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(gamma_neg2, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(gamma_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(gamma_negbig, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(gamma_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(gamma_neginf, (FLOAT_T)INFINITY, 0, 0),

#endif

	TEST(hypot_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
        TEST(hypot_1_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(hypot_qnan_1, (FLOAT_T)NAN, 0, 0),
        TEST(hypot_inf_qnan, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_neginf_qnan, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_qnan_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_qnan_neginf, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_snan_inf, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(hypot_snan_neginf, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(hypot_1_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_1_neginf, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_inf_1, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_neginf_1, (FLOAT_T)INFINITY, 0, 0),

#ifndef SIMPLE_MATH_ONLY

#ifndef NO_BESSEL_TESTS
        TEST(j0_inf, 0, 0, 0),
        TEST(j0_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(j0_snan, (FLOAT_T)NAN, FE_INVALID, 0),

        TEST(j1_inf, 0, 0, 0),
        TEST(j1_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(j1_snan, (FLOAT_T)NAN, FE_INVALID, 0),

        TEST(jn_inf, 0, 0, 0),
        TEST(jn_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(jn_snan, (FLOAT_T)NAN, FE_INVALID, 0),
#endif

#endif

        TEST(ldexp_1_0, (FLOAT_T)1.0, 0, 0),
        TEST(ldexp_qnan_0, (FLOAT_T)NAN, 0, 0),
        TEST(ldexp_snan_0, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(ldexp_inf_0, (FLOAT_T)INFINITY, 0, 0),
        TEST(ldexp_neginf_0, -(FLOAT_T)INFINITY, 0, 0),
        TEST(ldexp_1_negbig, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),
        TEST(ldexp_1_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),

#ifndef SIMPLE_MATH_ONLY

        TEST(lgamma_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(lgamma_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(lgamma_1, (FLOAT_T) 0.0, 0, 0),
        TEST(lgamma_2, (FLOAT_T) 0.0, 0, 0),
	TEST(lgamma_0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_neg0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_neg1, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_neg2, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(lgamma_negbig, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(lgamma_neginf, (FLOAT_T)INFINITY, 0, 0),

        TEST(lgamma_r_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(lgamma_r_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(lgamma_r_1, (FLOAT_T) 0.0, 0, 0),
        TEST(lgamma_r_2, (FLOAT_T) 0.0, 0, 0),
	TEST(lgamma_r_0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_neg0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_neg1, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_neg2, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
#if !defined(__PICOLIBC__) && defined(TEST_LONG_DOUBLE) && (defined(__x86_64) || defined(__i386))
        /* glibc on x86 gets this wrong */
	TEST(lgamma_r_negbig, (FLOAT_T)INFINITY, FE_DIVBYZERO|FE_OVERFLOW|FE_INEXACT, ERANGE),
#else
	TEST(lgamma_r_negbig, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
#endif
	TEST(lgamma_r_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(lgamma_r_neginf, (FLOAT_T)INFINITY, 0, 0),

	TEST(log_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(log_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(log_1, (FLOAT_T)0, 0, 0),
	TEST(log_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(log_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(log_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),

	TEST(log10_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(log10_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(log10_1, (FLOAT_T)0, 0, 0),
	TEST(log10_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(log10_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log10_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(log10_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),

	TEST(log1p_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(log1p_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(log1p_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(log1p_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(log1p_neg1, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log1p_neg2, (FLOAT_T)NAN, FE_INVALID, EDOM),

	TEST(log2_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(log2_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(log2_1, (FLOAT_T)0, 0, 0),
	TEST(log2_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(log2_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log2_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(log2_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),

#endif

        TEST(logb_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(logb_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(logb_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, 0),
        TEST(logb_neg0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, 0),
        TEST(logb_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(logb_neginf, (FLOAT_T)INFINITY, 0, 0),

#ifndef SIMPLE_MATH_ONLY

        TEST(nearbyint_qnan, (FLOAT_T) NAN, 0, 0),
        TEST(nearbyint_snan, (FLOAT_T) NAN, FE_INVALID, 0),
        TEST(nearbyint_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(nearbyint_neginf, -(FLOAT_T)INFINITY, 0, 0),
        TEST(nearbyint_big, BIG, 0, 0),
        TEST(nearbyint_negbig, -BIG, 0, 0),
        TEST(nearbyint_half, (FLOAT_T)0.0, 0, 0),
        TEST(nearbyint_neghalf, (FLOAT_T)-0.0, 0, 0),

	TEST(pow_neg_half, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(pow_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow_negbig, (FLOAT_T)-INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow_tiny, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),
        TEST(pow_1_1, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_2, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_neg2, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_inf, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_neginf, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_qnan, (FLOAT_T)1.0, 0, 0),
        TEST(pow_qnan_0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_qnan_neg0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(pow_snan_0, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(pow_snan_neg0, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(pow_1_0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_neg0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_0_0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_0_neg0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_inf_0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_inf_neg0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_0_odd, (FLOAT_T)0.0, 0, 0),
        TEST(pow_neg0_odd, (FLOAT_T)-0.0, 0, 0),
        TEST(pow_neg1_inf, (FLOAT_T)1.0, 0, 0),
        TEST(pow_neg1_neginf, (FLOAT_T)1.0, 0, 0),
        TEST(pow_half_neginf, (FLOAT_T)INFINITY, 0, 0),
        TEST(pow_neghalf_neginf, (FLOAT_T)INFINITY, 0, 0),
        TEST(pow_2_neginf, (FLOAT_T)0.0, 0, 0),
        TEST(pow_neg2_neginf, (FLOAT_T)0.0, 0, 0),
        TEST(pow_half_inf, (FLOAT_T)0.0, 0, 0),
        TEST(pow_neghalf_inf, (FLOAT_T)0.0, 0, 0),
        TEST(pow_2_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(pow_2_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(pow_2_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(pow_neg2_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(pow_neginf_negodd, (FLOAT_T)-0.0, 0, 0),
        TEST(pow_neginf_neghalf, (FLOAT_T)0.0, 0, 0),
        TEST(pow_neginf_neg2, (FLOAT_T)0.0, 0, 0),
        TEST(pow_neginf_odd, (FLOAT_T)-INFINITY, 0, 0),
        TEST(pow_neginf_half, (FLOAT_T)INFINITY, 0, 0),
        TEST(pow_neginf_2, (FLOAT_T)INFINITY, 0, 0),
        TEST(pow_inf_neg2, (FLOAT_T)0.0, 0, 0),
        TEST(pow_inf_2, (FLOAT_T)INFINITY, 0, 0),
	TEST(pow_0_neg2, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(pow_neg0_neg2, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(pow_0_neg3, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(pow_neg0_neg3, (FLOAT_T)-INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(pow_negsmall_negbigodd, (FLOAT_T)-INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow_negbig_bigodd, (FLOAT_T)-INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow_negsmall_negbigeven, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow_negbig_bigeven, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),

	TEST(pow10_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(pow10_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(pow10_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(pow10_neginf, (FLOAT_T)0.0, 0, 0),
	TEST(pow10_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow10_negbig, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),

	TEST(remainder_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(remainder_qnan_1, (FLOAT_T)NAN, 0, 0),
	TEST(remainder_1_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(remainder_snan_1, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(remainder_1_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(remainder_inf_2, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(remainder_inf_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(remainder_2_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(remainder_1_2, (FLOAT_T)1.0, 0, 0),
        TEST(remainder_neg1_2, -(FLOAT_T)1.0, 0, 0),
#endif

        TEST(rint_qnan, (FLOAT_T) NAN, 0, 0),
        TEST(rint_snan, (FLOAT_T) NAN, FE_INVALID, 0),
        TEST(rint_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(rint_neginf, -(FLOAT_T)INFINITY, 0, 0),
        TEST(rint_big, BIG, 0, 0),
        TEST(rint_negbig, -BIG, 0, 0),
        TEST(rint_half, (FLOAT_T) 0.0, FE_INEXACT, 0),
        TEST(rint_neghalf, (FLOAT_T)-0.0, FE_INEXACT, 0),

        TEST(scalb_1_1, (FLOAT_T)2.0, 0, 0),
        TEST(scalb_1_half, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(scalb_qnan_1, (FLOAT_T)NAN, 0, 0),
        TEST(scalb_1_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(scalb_snan_1, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(scalb_1_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(scalb_inf_2, (FLOAT_T)INFINITY, 0, 0),
        TEST(scalb_neginf_2, -(FLOAT_T)INFINITY, 0, 0),
        TEST(scalb_0_2, (FLOAT_T)0.0, 0, 0),
        TEST(scalb_neg0_2, (FLOAT_T)-0.0, 0, 0),
        TEST(scalb_0_inf, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(scalb_inf_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(scalb_neginf_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(scalb_1_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
        TEST(scalb_neg1_big, (FLOAT_T)-INFINITY, FE_OVERFLOW, ERANGE),
        TEST(scalb_1_negbig, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),
        TEST(scalb_neg1_negbig, (FLOAT_T)-0.0, FE_UNDERFLOW, ERANGE),
        TEST(scalbn_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
        TEST(scalbn_tiny, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),

#ifndef SIMPLE_MATH_ONLY

        TEST(nextafter_0_neg0, -(FLOAT_T)0, 0, 0),
        TEST(nextafter_neg0_0, (FLOAT_T)0, 0, 0),
        TEST(nextafter_0_1, (FLOAT_T) MIN_VAL, FE_UNDERFLOW, 0),
        TEST(nextafter_0_neg1, -(FLOAT_T) MIN_VAL, FE_UNDERFLOW, 0),
        TEST(nextafter_min_1, (FLOAT_T) MIN_VAL + (FLOAT_T) MIN_VAL, FE_UNDERFLOW, ERANGE),
        TEST(nextafter_negmin_neg1, -(FLOAT_T) MIN_VAL - (FLOAT_T) MIN_VAL, FE_UNDERFLOW, ERANGE),
        TEST(nextafter_qnan_1, (FLOAT_T)NAN, 0, 0),
        TEST(nextafter_snan_1, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(nextafter_max_inf, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
        TEST(nextafter_negmax_neginf, -(FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
        TEST(nextafter_min_0, (FLOAT_T)0, FE_UNDERFLOW, ERANGE),
        TEST(nextafter_negmin_0, (FLOAT_T)0, FE_UNDERFLOW, ERANGE),
        TEST(nextafter_1_2, (FLOAT_T)1.0 + EPSILON_VAL, 0, 0),
        TEST(nextafter_neg1_neg2, -(FLOAT_T)1.0 - EPSILON_VAL, 0, 0),

#if defined(__SIZEOF_LONG_DOUBLE__) && !defined(NO_NEXTTOWARD)
        TEST(nexttoward_0_neg0, -(FLOAT_T)0, 0, 0),
        TEST(nexttoward_neg0_0, (FLOAT_T)0, 0, 0),
        TEST(nexttoward_0_1, (FLOAT_T) MIN_VAL, FE_UNDERFLOW, 0),
        TEST(nexttoward_0_neg1, -(FLOAT_T) MIN_VAL, FE_UNDERFLOW, 0),
        TEST(nexttoward_min_1, (FLOAT_T) MIN_VAL + (FLOAT_T) MIN_VAL, FE_UNDERFLOW, ERANGE),
        TEST(nexttoward_negmin_neg1, -(FLOAT_T) MIN_VAL - (FLOAT_T) MIN_VAL, FE_UNDERFLOW, ERANGE),
        TEST(nexttoward_qnan_1, (FLOAT_T)NAN, 0, 0),
        TEST(nexttoward_snan_1, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(nexttoward_max_inf, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
        TEST(nexttoward_negmax_neginf, -(FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
        TEST(nexttoward_min_0, (FLOAT_T)0, FE_UNDERFLOW, ERANGE),
        TEST(nexttoward_negmin_0, (FLOAT_T)0, FE_UNDERFLOW, ERANGE),
        TEST(nexttoward_1_2, (FLOAT_T)1.0 + EPSILON_VAL, 0, 0),
        TEST(nexttoward_neg1_neg2, -(FLOAT_T)1.0 - EPSILON_VAL, 0, 0),
#endif

        TEST(sin_inf, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(sin_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(sin_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(sin_pio2, (FLOAT_T)1.0, FE_INEXACT, 0),
        TEST(sin_small, (FLOAT_T)SMALL, FE_INEXACT, 0),
        TEST(sin_0, (FLOAT_T)0.0, 0, 0),

        TEST(sincos, (FLOAT_T)1.0, 0, 0),
        TEST(sincos_inf, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(sincos_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(sincos_snan, (FLOAT_T)NAN, FE_INVALID, 0),

        TEST(sinh_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(sinh_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(sinh_0, (FLOAT_T)0.0, 0, 0),
        TEST(sinh_neg0, (FLOAT_T)-0.0, 0, 0),
        TEST(sinh_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(sinh_neginf, (FLOAT_T)-INFINITY, 0, 0),

#endif

	TEST(sqrt_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(sqrt_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(sqrt_0, (FLOAT_T)0.0, 0, 0),
        TEST(sqrt_neg0, (FLOAT_T)-0.0, 0, 0),
        TEST(sqrt_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(sqrt_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(sqrt_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
        /*
         * Picolibc doesn't ever set inexact for sqrt as that's
         * an expensive operation
         */
//	TEST(sqrt_2, (FLOAT_T)1.4142135623730951, FE_INEXACT, 0),

#ifndef SIMPLE_MATH_ONLY

        TEST(tan_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(tan_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(tan_inf, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(tan_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),
#if 0
        /* alas, we aren't close enough to π/2 to overflow */
        TEST(tan_pio2, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
#endif

        TEST(tanh_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(tanh_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(tanh_0, (FLOAT_T)0.0, 0, 0),
        TEST(tanh_neg0, (FLOAT_T)-0.0, 0, 0),
        TEST(tanh_inf, (FLOAT_T)1.0, 0, 0),
        TEST(tanh_neginf, (FLOAT_T)-1.0, 0, 0),

	TEST(tgamma_0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(tgamma_neg0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(tgamma_neg1, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(tgamma_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(tgamma_negbig, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(tgamma_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(tgamma_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(tgamma_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(tgamma_snan, (FLOAT_T)NAN, FE_INVALID, 0),
        /* ld128 tgamma sits atop lgamma which is not accurate for this value */
#if defined(__PICOLIBC__) && defined(TEST_LONG_DOUBLE) && __LDBL_MANT_DIG__ == 113
#else
	TEST(tgamma_small, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(tgamma_negsmall, -(FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
#endif

#ifndef NO_BESSEL_TESTS
        TEST(y0_inf, (FLOAT_T)0.0, 0, 0),
        TEST(y0_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(y0_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(y0_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(y0_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),

        TEST(y1_inf, (FLOAT_T)0.0, 0, 0),
        TEST(y1_qnan, (FLOAT_T)NAN, 0, 0),
        TEST(y1_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(y1_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(y1_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),

	TEST(yn_inf, (FLOAT_T)0.0, 0, 0),
	TEST(yn_qnan, (FLOAT_T)NAN, 0, 0),
	TEST(yn_snan, (FLOAT_T)NAN, FE_INVALID, 0),
	TEST(yn_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(yn_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
#endif
#endif

	{ 0 },
};

struct {
        long    (*func)(void);
	char	*name;
	long	value;
	int	except;
	int	errno_expect;
} makemathname(itests)[] = {
        TEST(fpclassify_snan, FP_NAN, 0, 0),
        TEST(fpclassify_nan, FP_NAN, 0, 0),
        TEST(fpclassify_inf, FP_INFINITE, 0, 0),
        TEST(fpclassify_neginf, FP_INFINITE, 0, 0),
        TEST(fpclassify_zero, FP_ZERO, 0, 0),
        TEST(fpclassify_negzero, FP_ZERO, 0, 0),
        TEST(fpclassify_small, FP_SUBNORMAL, 0, 0),
        TEST(fpclassify_negsmall, FP_SUBNORMAL, 0, 0),
        TEST(fpclassify_two, FP_NORMAL, 0, 0),
        TEST(fpclassify_negtwo, FP_NORMAL, 0, 0),

        TEST(isfinite_snan, 0, 0, 0),
        TEST(isfinite_nan, 0, 0, 0),
        TEST(isfinite_inf, 0, 0, 0),
        TEST(isfinite_neginf, 0, 0, 0),
        TEST(isfinite_zero, 1, 0, 0),
        TEST(isfinite_negzero, 1, 0, 0),
        TEST(isfinite_small, 1, 0, 0),
        TEST(isfinite_negsmall, 1, 0, 0),
        TEST(isfinite_two, 1, 0, 0),
        TEST(isfinite_negtwo, 1, 0, 0),

        TEST(isnormal_snan, 0, 0, 0),
        TEST(isnormal_nan, 0, 0, 0),
        TEST(isnormal_inf, 0, 0, 0),
        TEST(isnormal_neginf, 0, 0, 0),
        TEST(isnormal_zero, 0, 0, 0),
        TEST(isnormal_negzero, 0, 0, 0),
        TEST(isnormal_small, 0, 0, 0),
        TEST(isnormal_negsmall, 0, 0, 0),
        TEST(isnormal_two, 1, 0, 0),
        TEST(isnormal_negtwo, 1, 0, 0),

        TEST(isnan_snan, 1, 0, 0),
        TEST(isnan_nan, 1, 0, 0),
        TEST(isnan_inf, 0, 0, 0),
        TEST(isnan_neginf, 0, 0, 0),
        TEST(isnan_zero, 0, 0, 0),
        TEST(isnan_negzero, 0, 0, 0),
        TEST(isnan_small, 0, 0, 0),
        TEST(isnan_negsmall, 0, 0, 0),
        TEST(isnan_two, 0, 0, 0),
        TEST(isnan_negtwo, 0, 0, 0),

        TEST(isinf_snan, 0, 0, 0),
        TEST(isinf_nan, 0, 0, 0),
        TEST(isinf_inf, 1, 0, 0),
        TEST(isinf_neginf, 1, 0, 0),
        TEST(isinf_zero, 0, 0, 0),
        TEST(isinf_negzero, 0, 0, 0),
        TEST(isinf_small, 0, 0, 0),
        TEST(isinf_negsmall, 0, 0, 0),
        TEST(isinf_two, 0, 0, 0),
        TEST(isinf_negtwo, 0, 0, 0),

        TEST(ilogb_0, FP_ILOGB0, FE_INVALID, EDOM),
        TEST(ilogb_qnan, FP_ILOGBNAN, FE_INVALID, EDOM),
        TEST(ilogb_inf, INT_MAX, FE_INVALID, EDOM),
        TEST(ilogb_neginf, INT_MAX, FE_INVALID, EDOM),

        TEST(lrint_qnan, 0, FE_INVALID, 0),
        TEST(lrint_inf, 0, FE_INVALID, 0),
        TEST(lrint_neginf, 0, FE_INVALID, 0),
        TEST(lrint_big, 0, FE_INVALID, 0),
        TEST(lrint_negbig, 0, FE_INVALID, 0),

        TEST(lround_qnan, 0, FE_INVALID, 0),
        TEST(lround_inf, 0, FE_INVALID, 0),
        TEST(lround_neginf, 0, FE_INVALID, 0),
        TEST(lround_big, 0, FE_INVALID, 0),
        TEST(lround_negbig, 0, FE_INVALID, 0),
        { 0 },
};

#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
#define PRINT	if (!printed++) printf("    %-20.20s = %g errno %d (%s) except %s\n", \
                       makemathname(tests)[t].name, (double) v, err, strerror(err), e_to_str(except))
#else
#define PRINT	if (!printed++) printf("    %-20.20s = (float) errno %d (%s) except %s\n", \
                       makemathname(tests)[t].name, err, strerror(err), e_to_str(except))
#endif

#define IPRINT	if (!printed++) printf("    %-20.20s = %ld errno %d (%s) except %s\n", \
                       makemathname(itests)[t].name, iv, err, strerror(err), e_to_str(except))

int
makemathname(is_equal)(FLOAT_T a, FLOAT_T b)
{
    if (isinf(a) && isinf(b))
        return (a > 0) == (b > 0);
    if (makemathname(isnan)(a) && makemathname(isnan)(b))
        return issignaling(a) == issignaling(b);
    return a == b;
}

void
makemathname(print_float)(FLOAT_T a)
{
    if (makemathname(isnan)(a)) {
        if (issignaling(a))
            printf("sNaN");
        else
            printf("qNaN");
    } else
#if defined(TEST_LONG_DOUBLE) && !defined(__PICOLIBC__)
        printf("%.34g", (long double) a);
#else
        printf("%.17g", (double) a);
#endif
}

int
makemathname(run_tests)(void) {
	int result = 0;
	volatile FLOAT_T v;
        volatile long iv;
	int err, except;
	int t;
        int printed;

	for (t = 0; makemathname(tests)[t].func; t++) {
                printed = 0;
		errno = 0;
		feclearexcept(FE_ALL_EXCEPT);
		v = makemathname(tests)[t].func();
		err = errno;
		except = fetestexcept(MY_EXCEPT);
		if (!makemathname(is_equal)(v, makemathname(tests)[t].value))
		{
                        PRINT;
			printf("\tbad value got ");
                        makemathname(print_float)(v);
                        printf(" expect ");
                        makemathname(print_float)(makemathname(tests)[t].value);
                        printf("\n");
			++result;
		}
		if (math_errhandling & EXCEPTION_TEST) {
                    int expect_except = makemathname(tests)[t].except;
                    int mask = MY_EXCEPT;

                    /* underflow can be set for inexact operations */
                    if (expect_except & FE_INEXACT)
                        mask &= ~FE_UNDERFLOW;
                    else
                        mask &= ~FE_INEXACT;

                    if ((except & (expect_except | mask)) != expect_except) {
                                PRINT;
				printf("\texceptions supported. %s returns %s instead of %s\n",
                                       makemathname(tests)[t].name, e_to_str(except), e_to_str(makemathname(tests)[t].except));
				++result;
			}
		} else {
			if (except) {
                                PRINT;
				printf("\texceptions not supported. %s returns %s\n", makemathname(tests)[t].name, e_to_str(except));
				++result;
			}
		}
		if (math_errhandling & MATH_ERRNO) {
			if (err != makemathname(tests)[t].errno_expect) {
                                PRINT;
				printf("\terrno supported but %s returns %d (%s) instead of %d (%s)\n",
                                       makemathname(tests)[t].name, err, strerror(err),
                                       makemathname(tests)[t].errno_expect, strerror(makemathname(tests)[t].errno_expect));
				++result;
			}
		} else {
			if (err != 0) {
                                PRINT;
				printf("\terrno not supported but %s returns %d (%s)\n",
                                       makemathname(tests)[t].name, err, strerror(err));
				++result;
			}
		}
	}

	for (t = 0; makemathname(itests)[t].func; t++) {
                printed = 0;
		errno = 0;
		feclearexcept(FE_ALL_EXCEPT);
		iv = makemathname(itests)[t].func();
		err = errno;
		except = fetestexcept(MY_EXCEPT);
		if (iv != makemathname(itests)[t].value)
		{
                        IPRINT;
			printf("\tbad value got %ld expect %ld\n", iv, makemathname(itests)[t].value);
			++result;
		}
		if (math_errhandling & EXCEPTION_TEST) {
                        int expect_except = makemathname(itests)[t].except;
                        int mask = MY_EXCEPT;

                        /* underflow can be set for inexact operations */
                        if (expect_except & FE_INEXACT)
                                mask &= ~FE_UNDERFLOW;
                        else
                                mask &= ~FE_INEXACT;

                        if ((except & (expect_except | mask)) != expect_except) {
                                IPRINT;
				printf("\texceptions supported. %s returns %s instead of %s\n",
                                       makemathname(itests)[t].name, e_to_str(except), e_to_str(makemathname(itests)[t].except));
				++result;
			}
		} else {
			if (except) {
                                IPRINT;
				printf("\texceptions not supported. %s returns %s\n", makemathname(itests)[t].name, e_to_str(except));
				++result;
			}
		}
		if (math_errhandling & MATH_ERRNO) {
			if (err != makemathname(itests)[t].errno_expect) {
                                IPRINT;
				printf("\terrno supported but %s returns %d (%s)\n", makemathname(itests)[t].name, err, strerror(err));
				++result;
			}
		} else {
			if (err != 0) {
                                IPRINT;
				printf("\terrno not supported but %s returns %d (%s)\n", makemathname(itests)[t].name, err, strerror(err));
				++result;
			}
		}
	}
	return result;
}
