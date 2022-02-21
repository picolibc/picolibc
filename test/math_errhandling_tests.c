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
volatile FLOAT_T makemathname(three) = 3.0;
volatile FLOAT_T makemathname(half) = 0.5;
volatile FLOAT_T makemathname(big) = BIG;
volatile FLOAT_T makemathname(small) = SMALL;
volatile FLOAT_T makemathname(infval) = (FLOAT_T) INFINITY;
volatile FLOAT_T makemathname(minfval) = (FLOAT_T) -INFINITY;
volatile FLOAT_T makemathname(nanval) = (FLOAT_T) NAN;
volatile FLOAT_T makemathname(pio2) = (FLOAT_T) (M_PI/2.0);
volatile FLOAT_T makemathname(min_val) = (FLOAT_T) MIN_VAL;
volatile FLOAT_T makemathname(max_val) = (FLOAT_T) MAX_VAL;

FLOAT_T makemathname(scalb)(FLOAT_T, FLOAT_T);

#define cat2(a,b) a ## b
#define str(a) #a
#define TEST(n,v,ex,er)	{ .func = makemathname(cat2(test_, n)), .name = str(n), .value = (v), .except = (ex), .errno_expect = (er) }

static int _signgam;

FLOAT_T makemathname(test_acos_2)(void) { return makemathname(acos)(makemathname(two)); }
FLOAT_T makemathname(test_acos_nan)(void) { return makemathname(acos)(makemathname(nanval)); }
FLOAT_T makemathname(test_acos_inf)(void) { return makemathname(acos)(makemathname(infval)); }
FLOAT_T makemathname(test_acos_minf)(void) { return makemathname(acos)(makemathname(minfval)); }

FLOAT_T makemathname(test_acosh_half)(void) { return makemathname(acosh)(makemathname(one)/makemathname(two)); }
FLOAT_T makemathname(test_acosh_nan)(void) { return makemathname(acosh)(makemathname(nanval)); }
FLOAT_T makemathname(test_acosh_inf)(void) { return makemathname(acosh)(makemathname(infval)); }
FLOAT_T makemathname(test_acosh_minf)(void) { return makemathname(acosh)(makemathname(minfval)); }

FLOAT_T makemathname(test_asin_2)(void) { return makemathname(asin)(makemathname(two)); }
FLOAT_T makemathname(test_asin_nan)(void) { return makemathname(asin)(makemathname(nanval)); }
FLOAT_T makemathname(test_asin_inf)(void) { return makemathname(asin)(makemathname(infval)); }
FLOAT_T makemathname(test_asin_minf)(void) { return makemathname(asin)(makemathname(minfval)); }

FLOAT_T makemathname(test_asinh_0)(void) { return makemathname(asinh)(makemathname(zero)); }
FLOAT_T makemathname(test_asinh_neg0)(void) { return makemathname(asinh)(makemathname(negzero)); }
FLOAT_T makemathname(test_asinh_nan)(void) { return makemathname(asinh)(makemathname(nanval)); }
FLOAT_T makemathname(test_asinh_inf)(void) { return makemathname(asinh)(makemathname(infval)); }
FLOAT_T makemathname(test_asinh_minf)(void) { return makemathname(asinh)(makemathname(minfval)); }

FLOAT_T makemathname(test_atan2_nanx)(void) { return makemathname(atan2)(makemathname(one), makemathname(nanval)); }
FLOAT_T makemathname(test_atan2_nany)(void) { return makemathname(atan2)(makemathname(nanval), makemathname(one)); }
FLOAT_T makemathname(test_atan2_tiny)(void) { return makemathname(atan2)(makemathname(min_val), makemathname(max_val)); }
FLOAT_T makemathname(test_atan2_nottiny)(void) { return makemathname(atan2)(makemathname(min_val), (FLOAT_T) -0x8p-20); }

FLOAT_T makemathname(test_atanh_nan)(void) { return makemathname(atanh)(makemathname(nanval)); }
FLOAT_T makemathname(test_atanh_1)(void) { return makemathname(atanh)(makemathname(one)); }
FLOAT_T makemathname(test_atanh_neg1)(void) { return makemathname(atanh)(-makemathname(one)); }
FLOAT_T makemathname(test_atanh_2)(void) { return makemathname(atanh)(makemathname(two)); }
FLOAT_T makemathname(test_atanh_neg2)(void) { return makemathname(atanh)(-makemathname(two)); }

FLOAT_T makemathname(test_cbrt_0)(void) { return makemathname(cbrt)(makemathname(zero)); }
FLOAT_T makemathname(test_cbrt_neg0)(void) { return makemathname(cbrt)(-makemathname(zero)); }
FLOAT_T makemathname(test_cbrt_inf)(void) { return makemathname(cbrt)(makemathname(infval)); }
FLOAT_T makemathname(test_cbrt_neginf)(void) { return makemathname(cbrt)(-makemathname(infval)); }
FLOAT_T makemathname(test_cbrt_nan)(void) { return makemathname(cbrt)(makemathname(nanval)); }

FLOAT_T makemathname(test_cos_inf)(void) { return makemathname(cos)(makemathname(infval)); }
FLOAT_T makemathname(test_cos_nan)(void) { return makemathname(cos)(makemathname(nanval)); }
FLOAT_T makemathname(test_cos_0)(void) { return makemathname(cos)(makemathname(zero)); }

FLOAT_T makemathname(test_cosh_big)(void) { return makemathname(cosh)(makemathname(big)); }
FLOAT_T makemathname(test_cosh_negbig)(void) { return makemathname(cosh)(makemathname(big)); }

FLOAT_T makemathname(test_drem_0)(void) { return makemathname(drem)(makemathname(two), makemathname(zero)); }
FLOAT_T makemathname(test_drem_nan_1)(void) { return makemathname(drem)(makemathname(nanval), makemathname(one)); }
FLOAT_T makemathname(test_drem_1_nan)(void) { return makemathname(drem)(makemathname(one), makemathname(nanval)); }
FLOAT_T makemathname(test_drem_inf_2)(void) { return makemathname(drem)(makemathname(infval), makemathname(two)); }
FLOAT_T makemathname(test_drem_inf_0)(void) { return makemathname(drem)(makemathname(infval), makemathname(zero)); }
FLOAT_T makemathname(test_drem_2_0)(void) { return makemathname(drem)(makemathname(two), makemathname(zero)); }
FLOAT_T makemathname(test_drem_1_2)(void) { return makemathname(drem)(makemathname(one), makemathname(two)); }
FLOAT_T makemathname(test_drem_neg1_2)(void) { return makemathname(drem)(-makemathname(one), makemathname(two)); }

FLOAT_T makemathname(test_erf_nan)(void) { return makemathname(erf)(makemathname(nanval)); }
FLOAT_T makemathname(test_erf_0)(void) { return makemathname(erf)(makemathname(zero)); }
FLOAT_T makemathname(test_erf_neg0)(void) { return makemathname(erf)(-makemathname(zero)); }
FLOAT_T makemathname(test_erf_inf)(void) { return makemathname(erf)(makemathname(infval)); }
FLOAT_T makemathname(test_erf_neginf)(void) { return makemathname(erf)(-makemathname(infval)); }
FLOAT_T makemathname(test_erf_small)(void) { return makemathname(erf)(makemathname(small)); }

FLOAT_T makemathname(test_exp_nan)(void) { return makemathname(exp)(makemathname(nanval)); }
FLOAT_T makemathname(test_exp_inf)(void) { return makemathname(exp)(makemathname(infval)); }
FLOAT_T makemathname(test_exp_neginf)(void) { return makemathname(exp)(-makemathname(infval)); }
FLOAT_T makemathname(test_exp_big)(void) { return makemathname(exp)(makemathname(big)); }
FLOAT_T makemathname(test_exp_negbig)(void) { return makemathname(exp)(-makemathname(big)); }

FLOAT_T makemathname(test_exp2_nan)(void) { return makemathname(exp2)(makemathname(nanval)); }
FLOAT_T makemathname(test_exp2_inf)(void) { return makemathname(exp2)(makemathname(infval)); }
FLOAT_T makemathname(test_exp2_neginf)(void) { return makemathname(exp2)(-makemathname(infval)); }
FLOAT_T makemathname(test_exp2_big)(void) { return makemathname(exp2)(makemathname(big)); }
FLOAT_T makemathname(test_exp2_negbig)(void) { return makemathname(exp2)(-makemathname(big)); }

FLOAT_T makemathname(test_exp10_nan)(void) { return makemathname(exp10)(makemathname(nanval)); }
FLOAT_T makemathname(test_exp10_inf)(void) { return makemathname(exp10)(makemathname(infval)); }
FLOAT_T makemathname(test_exp10_neginf)(void) { return makemathname(exp10)(-makemathname(infval)); }
FLOAT_T makemathname(test_exp10_big)(void) { return makemathname(exp10)(makemathname(big)); }
FLOAT_T makemathname(test_exp10_negbig)(void) { return makemathname(exp10)(-makemathname(big)); }

FLOAT_T makemathname(test_expm1_nan)(void) { return makemathname(expm1)(makemathname(nanval)); }
FLOAT_T makemathname(test_expm1_0)(void) { return makemathname(expm1)(makemathname(zero)); }
FLOAT_T makemathname(test_expm1_neg0)(void) { return makemathname(expm1)(-makemathname(zero)); }
FLOAT_T makemathname(test_expm1_inf)(void) { return makemathname(expm1)(makemathname(infval)); }
FLOAT_T makemathname(test_expm1_neginf)(void) { return makemathname(expm1)(-makemathname(infval)); }
FLOAT_T makemathname(test_expm1_big)(void) { return makemathname(expm1)(makemathname(big)); }
FLOAT_T makemathname(test_expm1_negbig)(void) { return makemathname(expm1)(-makemathname(big)); }

FLOAT_T makemathname(test_fabs_nan)(void) { return makemathname(fabs)(makemathname(nanval)); }
FLOAT_T makemathname(test_fabs_0)(void) { return makemathname(fabs)(makemathname(zero)); }
FLOAT_T makemathname(test_fabs_neg0)(void) { return makemathname(fabs)(-makemathname(zero)); }
FLOAT_T makemathname(test_fabs_inf)(void) { return makemathname(fabs)(makemathname(infval)); }
FLOAT_T makemathname(test_fabs_neginf)(void) { return makemathname(fabs)(-makemathname(infval)); }

FLOAT_T makemathname(test_fdim_nan_1)(void) { return makemathname(fdim)(makemathname(nanval), makemathname(one)); }
FLOAT_T makemathname(test_fdim_1_nan)(void) { return makemathname(fdim)(makemathname(one), makemathname(nanval)); }
FLOAT_T makemathname(test_fdim_inf_1)(void) { return makemathname(fdim)(makemathname(infval), makemathname(one)); }
FLOAT_T makemathname(test_fdim_1_inf)(void) { return makemathname(fdim)(makemathname(one), makemathname(infval)); }
FLOAT_T makemathname(test_fdim_big_negbig)(void) { return makemathname(fdim)(makemathname(big), -makemathname(big)); }

FLOAT_T makemathname(test_floor_1)(void) { return makemathname(floor)(makemathname(one)); }
FLOAT_T makemathname(test_floor_0)(void) { return makemathname(floor)(makemathname(zero)); }
FLOAT_T makemathname(test_floor_neg0)(void) { return makemathname(floor)(-makemathname(zero)); }
FLOAT_T makemathname(test_floor_nan)(void) { return makemathname(floor)(makemathname(nanval)); }
FLOAT_T makemathname(test_floor_inf)(void) { return makemathname(floor)(makemathname(infval)); }
FLOAT_T makemathname(test_floor_neginf)(void) { return makemathname(floor)(-makemathname(infval)); }

FLOAT_T makemathname(test_fma_big_big_1)(void) { return makemathname(fma)(makemathname(big), makemathname(big), makemathname(one)); }
FLOAT_T makemathname(test_fma_big_negbig_1)(void) { return makemathname(fma)(makemathname(big), -makemathname(big), makemathname(one)); }
FLOAT_T makemathname(test_fma_small_small_1)(void) { return makemathname(fma)(makemathname(small), makemathname(small), makemathname(one)); }
FLOAT_T makemathname(test_fma_small_negsmall_1)(void) { return makemathname(fma)(makemathname(small), -makemathname(small), makemathname(one)); }
FLOAT_T makemathname(test_fma_small_small_0)(void) { return makemathname(fma)(makemathname(small), makemathname(small), makemathname(zero)); }
FLOAT_T makemathname(test_fma_small_negsmall_0)(void) { return makemathname(fma)(makemathname(small), -makemathname(small), makemathname(zero)); }
FLOAT_T makemathname(test_fma_nan_1_1)(void) { return makemathname(fma)(makemathname(nanval), makemathname(one), makemathname(one)); }
FLOAT_T makemathname(test_fma_1_nan_1)(void) { return makemathname(fma)(makemathname(one), makemathname(nanval), makemathname(one)); }
FLOAT_T makemathname(test_fma_1_1_nan)(void) { return makemathname(fma)(makemathname(one), makemathname(one), makemathname(nanval)); }
FLOAT_T makemathname(test_fma_inf_1_neginf)(void) { return makemathname(fma)(makemathname(infval), makemathname(one), -makemathname(infval)); }
FLOAT_T makemathname(test_fma_1_inf_neginf)(void) { return makemathname(fma)(makemathname(one), makemathname(infval), -makemathname(infval)); }
FLOAT_T makemathname(test_fma_neginf_1_inf)(void) { return makemathname(fma)(makemathname(one), -makemathname(infval), makemathname(infval)); }
FLOAT_T makemathname(test_fma_1_neginf_inf)(void) { return makemathname(fma)(-makemathname(infval), makemathname(one), makemathname(infval)); }
FLOAT_T makemathname(test_fma_inf_0_1)(void) { return makemathname(fma)(makemathname(infval), makemathname(zero), makemathname(one)); }
FLOAT_T makemathname(test_fma_0_inf_1)(void) { return makemathname(fma)(makemathname(zero), makemathname(infval), makemathname(one)); }
FLOAT_T makemathname(test_fma_inf_0_nan)(void) { return makemathname(fma)(makemathname(infval), makemathname(zero), makemathname(nanval)); }
FLOAT_T makemathname(test_fma_0_inf_nan)(void) { return makemathname(fma)(makemathname(zero), makemathname(infval), makemathname(nanval)); }

FLOAT_T makemathname(test_fmax_nan_nan)(void) { return makemathname(fmax)(makemathname(nanval), makemathname(nanval)); }
FLOAT_T makemathname(test_fmax_nan_1)(void) { return makemathname(fmax)(makemathname(nanval), makemathname(one)); }
FLOAT_T makemathname(test_fmax_1_nan)(void) { return makemathname(fmax)(makemathname(one), makemathname(nanval)); }

FLOAT_T makemathname(test_fmin_nan_nan)(void) { return makemathname(fmin)(makemathname(nanval), makemathname(nanval)); }
FLOAT_T makemathname(test_fmin_nan_1)(void) { return makemathname(fmin)(makemathname(nanval), makemathname(one)); }
FLOAT_T makemathname(test_fmin_1_nan)(void) { return makemathname(fmin)(makemathname(one), makemathname(nanval)); }

FLOAT_T makemathname(test_fmod_nan_1)(void) { return makemathname(fmod)(makemathname(nanval), makemathname(one)); }
FLOAT_T makemathname(test_fmod_1_nan)(void) { return makemathname(fmod)(makemathname(one), makemathname(nanval)); }
FLOAT_T makemathname(test_fmod_inf_1)(void) { return makemathname(fmod)(makemathname(infval), makemathname(one)); }
FLOAT_T makemathname(test_fmod_neginf_1)(void) { return makemathname(fmod)(-makemathname(infval), makemathname(one)); }
FLOAT_T makemathname(test_fmod_1_0)(void) { return makemathname(fmod)(makemathname(one), makemathname(zero)); }
FLOAT_T makemathname(test_fmod_0_1)(void) { return makemathname(fmod)(makemathname(zero), makemathname(one)); }
FLOAT_T makemathname(test_fmod_neg0_1)(void) { return makemathname(fmod)(-makemathname(zero), makemathname(one)); }

FLOAT_T makemathname(test_hypot_big)(void) { return makemathname(hypot)(makemathname(big), makemathname(big)); }
FLOAT_T makemathname(test_hypot_1_nan)(void) { return makemathname(hypot)(makemathname(one), makemathname(nanval)); }
FLOAT_T makemathname(test_hypot_nan_1)(void) { return makemathname(hypot)(makemathname(nanval), makemathname(one)); }
FLOAT_T makemathname(test_hypot_inf_nan)(void) { return makemathname(hypot)(makemathname(infval), makemathname(nanval)); }
FLOAT_T makemathname(test_hypot_neginf_nan)(void) { return makemathname(hypot)(-makemathname(infval), makemathname(nanval)); }
FLOAT_T makemathname(test_hypot_nan_inf)(void) { return makemathname(hypot)(makemathname(nanval), makemathname(infval)); }
FLOAT_T makemathname(test_hypot_nan_neginf)(void) { return makemathname(hypot)(makemathname(nanval), -makemathname(infval)); }
FLOAT_T makemathname(test_hypot_1_inf)(void) { return makemathname(hypot)(makemathname(one), makemathname(infval)); }
FLOAT_T makemathname(test_hypot_1_neginf)(void) { return makemathname(hypot)(makemathname(one), -makemathname(infval)); }
FLOAT_T makemathname(test_hypot_inf_1)(void) { return makemathname(hypot)(makemathname(infval), makemathname(one)); }
FLOAT_T makemathname(test_hypot_neginf_1)(void) { return makemathname(hypot)(-makemathname(infval), makemathname(one)); }

long makemathname(test_ilogb_0)(void) { return makemathname(ilogb)(makemathname(zero)); }
long makemathname(test_ilogb_nan)(void) { return makemathname(ilogb)(makemathname(nanval)); }
long makemathname(test_ilogb_inf)(void) { return makemathname(ilogb)(makemathname(infval)); }
long makemathname(test_ilogb_neginf)(void) { return makemathname(ilogb)(-makemathname(infval)); }

FLOAT_T makemathname(test_j0_inf)(void) { return makemathname(j0)(makemathname(infval)); }
FLOAT_T makemathname(test_j0_nan)(void) { return makemathname(j0)(makemathname(nanval)); }

FLOAT_T makemathname(test_j1_inf)(void) { return makemathname(j1)(makemathname(infval)); }
FLOAT_T makemathname(test_j1_nan)(void) { return makemathname(j1)(makemathname(nanval)); }

FLOAT_T makemathname(test_jn_inf)(void) { return makemathname(jn)(3, makemathname(infval)); }
FLOAT_T makemathname(test_jn_nan)(void) { return makemathname(jn)(3, makemathname(nanval)); }

FLOAT_T makemathname(test_ldexp_1_0)(void) { return makemathname(ldexp)(makemathname(one), 0); }
FLOAT_T makemathname(test_ldexp_nan_0)(void) { return makemathname(ldexp)(makemathname(nanval), 0); }
FLOAT_T makemathname(test_ldexp_inf_0)(void) { return makemathname(ldexp)(makemathname(infval), 0); }
FLOAT_T makemathname(test_ldexp_neginf_0)(void) { return makemathname(ldexp)(-makemathname(infval), 0); }
FLOAT_T makemathname(test_ldexp_1_negbig)(void) { return makemathname(ldexp)(makemathname(one), -(__DBL_MAX_EXP__ * 100)); }
FLOAT_T makemathname(test_ldexp_1_big)(void) { return makemathname(ldexp)(makemathname(one),(__DBL_MAX_EXP__ * 100)); }

long makemathname(test_lrint_nan)(void) { makemathname(lrint)(makemathname(nanval)); return 0; }
long makemathname(test_lrint_inf)(void) { makemathname(lrint)(makemathname(infval)); return 0; }
long makemathname(test_lrint_neginf)(void) { makemathname(lrint)(-makemathname(infval)); return 0; }
long makemathname(test_lrint_big)(void) { makemathname(lrint)(makemathname(big)); return 0; }
long makemathname(test_lrint_negbig)(void) { makemathname(lrint)(-makemathname(big)); return 0; }

long makemathname(test_lround_nan)(void) { makemathname(lround)(makemathname(nanval)); return 0; }
long makemathname(test_lround_inf)(void) { makemathname(lround)(makemathname(infval)); return 0; }
long makemathname(test_lround_neginf)(void) { makemathname(lround)(-makemathname(infval)); return 0; }
long makemathname(test_lround_big)(void) { makemathname(lround)(makemathname(big)); return 0; }
long makemathname(test_lround_negbig)(void) { makemathname(lround)(-makemathname(big)); return 0; }

FLOAT_T makemathname(test_sin_inf)(void) { return makemathname(sin)(makemathname(infval)); }
FLOAT_T makemathname(test_sin_nan)(void) { return makemathname(sin)(makemathname(nanval)); }
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
FLOAT_T makemathname(test_sincos_nan)(void) { FLOAT_T s, c; makemathname(sincos)(makemathname(nanval), &s, &c); return s + c; }

FLOAT_T makemathname(test_sinh_nan)(void) { return makemathname(sinh)(makemathname(nanval)); }
FLOAT_T makemathname(test_sinh_0)(void) { return makemathname(sinh)(makemathname(zero)); }
FLOAT_T makemathname(test_sinh_neg0)(void) { return makemathname(sinh)(-makemathname(zero)); }
FLOAT_T makemathname(test_sinh_inf)(void) { return makemathname(sinh)(makemathname(infval)); }
FLOAT_T makemathname(test_sinh_neginf)(void) { return makemathname(sinh)(-makemathname(infval)); }

FLOAT_T makemathname(test_tgamma_0)(void) { return makemathname(tgamma)(makemathname(zero)); }
FLOAT_T makemathname(test_tgamma_neg0)(void) { return makemathname(tgamma)(makemathname(negzero)); }
FLOAT_T makemathname(test_tgamma_neg1)(void) { return makemathname(tgamma)(-makemathname(one)); }
FLOAT_T makemathname(test_tgamma_big)(void) { return makemathname(tgamma)(makemathname(big)); }
FLOAT_T makemathname(test_tgamma_negbig)(void) { return makemathname(tgamma)(makemathname(-big)); }
FLOAT_T makemathname(test_tgamma_inf)(void) { return makemathname(tgamma)(makemathname(infval)); }
FLOAT_T makemathname(test_tgamma_neginf)(void) { return makemathname(tgamma)(-makemathname(infval)); }
FLOAT_T makemathname(test_tgamma_small)(void) { return makemathname(tgamma)(small); }
FLOAT_T makemathname(test_tgamma_negsmall)(void) { return makemathname(tgamma)(-small); }

FLOAT_T makemathname(test_lgamma_nan)(void) { return makemathname(lgamma)(makemathname(nanval)); }
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

FLOAT_T makemathname(test_lgamma_r_nan)(void) { return makemathname_r(lgamma)(makemathname(nanval), &_signgam); }
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

FLOAT_T makemathname(test_gamma_nan)(void) { return makemathname(gamma)(makemathname(nanval)); }
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

FLOAT_T makemathname(test_log_nan)(void) { return makemathname(log)(makemathname(nanval)); }
FLOAT_T makemathname(test_log_1)(void) { return makemathname(log)(makemathname(one)); }
FLOAT_T makemathname(test_log_inf)(void) { return makemathname(log)(makemathname(infval)); }
FLOAT_T makemathname(test_log_0)(void) { return makemathname(log)(makemathname(zero)); }
FLOAT_T makemathname(test_log_neg)(void) { return makemathname(log)(-makemathname(one)); }
FLOAT_T makemathname(test_log_neginf)(void) { return makemathname(log)(-makemathname(one)); }

FLOAT_T makemathname(test_log10_nan)(void) { return makemathname(log10)(makemathname(nanval)); }
FLOAT_T makemathname(test_log10_1)(void) { return makemathname(log10)(makemathname(one)); }
FLOAT_T makemathname(test_log10_inf)(void) { return makemathname(log10)(makemathname(infval)); }
FLOAT_T makemathname(test_log10_0)(void) { return makemathname(log10)(makemathname(zero)); }
FLOAT_T makemathname(test_log10_neg)(void) { return makemathname(log10)(-makemathname(one)); }
FLOAT_T makemathname(test_log10_neginf)(void) { return makemathname(log10)(-makemathname(one)); }

FLOAT_T makemathname(test_log1p_nan)(void) { return makemathname(log1p)(makemathname(nanval)); }
FLOAT_T makemathname(test_log1p_inf)(void) { return makemathname(log1p)(makemathname(infval)); }
FLOAT_T makemathname(test_log1p_neginf)(void) { return makemathname(log1p)(-makemathname(infval)); }
FLOAT_T makemathname(test_log1p_neg1)(void) { return makemathname(log1p)(-makemathname(one)); }
FLOAT_T makemathname(test_log1p_neg2)(void) { return makemathname(log1p)(-makemathname(two)); }

FLOAT_T makemathname(test_log2_nan)(void) { return makemathname(log2)(makemathname(nanval)); }
FLOAT_T makemathname(test_log2_1)(void) { return makemathname(log2)(makemathname(one)); }
FLOAT_T makemathname(test_log2_inf)(void) { return makemathname(log2)(makemathname(infval)); }
FLOAT_T makemathname(test_log2_0)(void) { return makemathname(log2)(makemathname(zero)); }
FLOAT_T makemathname(test_log2_neg)(void) { return makemathname(log2)(-makemathname(one)); }
FLOAT_T makemathname(test_log2_neginf)(void) { return makemathname(log2)(-makemathname(one)); }

FLOAT_T makemathname(test_logb_nan)(void) { return makemathname(logb)(makemathname(nanval)); }
FLOAT_T makemathname(test_logb_0)(void) { return makemathname(logb)(makemathname(zero)); }
FLOAT_T makemathname(test_logb_neg0)(void) { return makemathname(logb)(-makemathname(zero)); }
FLOAT_T makemathname(test_logb_inf)(void) { return makemathname(logb)(makemathname(infval)); }
FLOAT_T makemathname(test_logb_neginf)(void) { return makemathname(logb)(-makemathname(infval)); }

FLOAT_T makemathname(test_pow_neg_half)(void) { return makemathname(pow)(-makemathname(two), makemathname(half)); }
FLOAT_T makemathname(test_pow_big)(void) { return makemathname(pow)(makemathname(two), makemathname(big)); }
FLOAT_T makemathname(test_pow_negbig)(void) { return makemathname(pow)(-makemathname(big), makemathname(three)); }
FLOAT_T makemathname(test_pow_tiny)(void) { return makemathname(pow)(makemathname(two), -makemathname(big)); }
FLOAT_T makemathname(test_pow_1_1)(void) { return makemathname(pow)(makemathname(one), makemathname(one)); }
FLOAT_T makemathname(test_pow_1_2)(void) { return makemathname(pow)(makemathname(one), makemathname(two)); }
FLOAT_T makemathname(test_pow_1_neg2)(void) { return makemathname(pow)(makemathname(one), makemathname(two)); }
FLOAT_T makemathname(test_pow_1_inf)(void) { return makemathname(pow)(makemathname(one), makemathname(infval)); }
FLOAT_T makemathname(test_pow_1_neginf)(void) { return makemathname(pow)(makemathname(one), -makemathname(infval)); }
FLOAT_T makemathname(test_pow_1_nan)(void) { return makemathname(pow)(makemathname(one), makemathname(nanval)); }
FLOAT_T makemathname(test_pow_1_0)(void) { return makemathname(pow)(makemathname(one), makemathname(zero)); }
FLOAT_T makemathname(test_pow_1_neg0)(void) { return makemathname(pow)(makemathname(one), -makemathname(zero)); }
FLOAT_T makemathname(test_pow_0_0)(void) { return makemathname(pow)(makemathname(zero), makemathname(zero)); }
FLOAT_T makemathname(test_pow_0_neg0)(void) { return makemathname(pow)(makemathname(zero), -makemathname(zero)); }
FLOAT_T makemathname(test_pow_inf_0)(void) { return makemathname(pow)(makemathname(infval), makemathname(zero)); }
FLOAT_T makemathname(test_pow_inf_neg0)(void) { return makemathname(pow)(makemathname(infval), -makemathname(zero)); }
FLOAT_T makemathname(test_pow_nan_0)(void) { return makemathname(pow)(makemathname(nanval), makemathname(zero)); }
FLOAT_T makemathname(test_pow_nan_neg0)(void) { return makemathname(pow)(makemathname(nanval), -makemathname(zero)); }
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

#ifndef __PICOLIBC__
#define pow10(x) exp10(x)
#define pow10f(x) exp10f(x)
#endif

FLOAT_T makemathname(test_pow10_nan)(void) { return makemathname(pow10)(makemathname(nanval)); }
FLOAT_T makemathname(test_pow10_inf)(void) { return makemathname(pow10)(makemathname(infval)); }
FLOAT_T makemathname(test_pow10_neginf)(void) { return makemathname(pow10)(-makemathname(infval)); }
FLOAT_T makemathname(test_pow10_big)(void) { return makemathname(pow10)(makemathname(big)); }
FLOAT_T makemathname(test_pow10_negbig)(void) { return makemathname(pow10)(-makemathname(big)); }

FLOAT_T makemathname(test_remainder_0)(void) { return makemathname(remainder)(makemathname(two), makemathname(zero)); }
FLOAT_T makemathname(test_remainder_nan_1)(void) { return makemathname(remainder)(makemathname(nanval), makemathname(one)); }
FLOAT_T makemathname(test_remainder_1_nan)(void) { return makemathname(remainder)(makemathname(one), makemathname(nanval)); }
FLOAT_T makemathname(test_remainder_inf_2)(void) { return makemathname(remainder)(makemathname(infval), makemathname(two)); }
FLOAT_T makemathname(test_remainder_inf_0)(void) { return makemathname(remainder)(makemathname(infval), makemathname(zero)); }
FLOAT_T makemathname(test_remainder_2_0)(void) { return makemathname(remainder)(makemathname(two), makemathname(zero)); }
FLOAT_T makemathname(test_remainder_1_2)(void) { return makemathname(remainder)(makemathname(one), makemathname(two)); }
FLOAT_T makemathname(test_remainder_neg1_2)(void) { return makemathname(remainder)(-makemathname(one), makemathname(two)); }

FLOAT_T makemathname(test_sqrt_nan)(void) { return makemathname(sqrt)(makemathname(nanval)); }
FLOAT_T makemathname(test_sqrt_0)(void) { return makemathname(sqrt)(makemathname(zero)); }
FLOAT_T makemathname(test_sqrt_neg0)(void) { return makemathname(sqrt)(-makemathname(zero)); }
FLOAT_T makemathname(test_sqrt_inf)(void) { return makemathname(sqrt)(makemathname(infval)); }
FLOAT_T makemathname(test_sqrt_neginf)(void) { return makemathname(sqrt)(-makemathname(infval)); }
FLOAT_T makemathname(test_sqrt_neg)(void) { return makemathname(sqrt)(-makemathname(two)); }
FLOAT_T makemathname(test_sqrt_2)(void) { return makemathname(sqrt)(makemathname(two)); }

FLOAT_T makemathname(test_tan_nan)(void) { return makemathname(tan)(makemathname(nanval)); }
FLOAT_T makemathname(test_tan_inf)(void) { return makemathname(tan)(makemathname(infval)); }
FLOAT_T makemathname(test_tan_neginf)(void) { return makemathname(tan)(-makemathname(infval)); }
FLOAT_T makemathname(test_tan_pio2)(void) { return makemathname(tan)(makemathname(pio2)); }

FLOAT_T makemathname(test_tanh_nan)(void) { return makemathname(tanh)(makemathname(nanval)); }
FLOAT_T makemathname(test_tanh_0)(void) { return makemathname(tanh)(makemathname(zero)); }
FLOAT_T makemathname(test_tanh_neg0)(void) { return makemathname(tanh)(-makemathname(zero)); }
FLOAT_T makemathname(test_tanh_inf)(void) { return makemathname(tanh)(makemathname(infval)); }
FLOAT_T makemathname(test_tanh_neginf)(void) { return makemathname(tanh)(-makemathname(infval)); }

FLOAT_T makemathname(test_y0_nan)(void) { return makemathname(y0)(makemathname(nanval)); }
FLOAT_T makemathname(test_y0_neg)(void) { return makemathname(y0)(-makemathname(one)); }
FLOAT_T makemathname(test_y0_0)(void) { return makemathname(y0)(makemathname(zero)); }

FLOAT_T makemathname(test_y1_nan)(void) { return makemathname(y1)(makemathname(nanval)); }
FLOAT_T makemathname(test_y1_neg)(void) { return makemathname(y1)(-makemathname(one)); }
FLOAT_T makemathname(test_y1_0)(void) { return makemathname(y1)(makemathname(zero)); }

FLOAT_T makemathname(test_yn_nan)(void) { return makemathname(yn)(2, makemathname(nanval)); }
FLOAT_T makemathname(test_yn_neg)(void) { return makemathname(yn)(2, -makemathname(one)); }
FLOAT_T makemathname(test_yn_0)(void) { return makemathname(yn)(2, makemathname(zero)); }

FLOAT_T makemathname(test_scalb_1_1)(void) { return makemathname(scalb)(makemathname(one), makemathname(one)); }
FLOAT_T makemathname(test_scalb_1_half)(void) { return makemathname(scalb)(makemathname(one), makemathname(half)); }
FLOAT_T makemathname(test_scalb_nan_1)(void) { return makemathname(scalb)(makemathname(nanval), makemathname(one)); }
FLOAT_T makemathname(test_scalb_1_nan)(void) { return makemathname(scalb)(makemathname(one), makemathname(nanval)); }
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

struct {
	FLOAT_T	(*func)(void);
	char	*name;
	FLOAT_T	value;
	int	except;
	int	errno_expect;
} makemathname(tests)[] = {
	TEST(acos_2, (FLOAT_T) NAN, FE_INVALID, EDOM),
	TEST(acos_nan, (FLOAT_T) NAN, 0, 0),
	TEST(acos_inf, (FLOAT_T) NAN, FE_INVALID, EDOM),
	TEST(acos_minf, (FLOAT_T) NAN, FE_INVALID, EDOM),

	TEST(acosh_half, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(acosh_nan, (FLOAT_T)NAN, 0, 0),
	TEST(acosh_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(acosh_minf, (FLOAT_T)NAN, FE_INVALID, EDOM),

	TEST(asin_2, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(asin_nan, (FLOAT_T) NAN, 0, 0),
	TEST(asin_inf, (FLOAT_T) NAN, FE_INVALID, EDOM),
	TEST(asin_minf, (FLOAT_T) NAN, FE_INVALID, EDOM),

        TEST(asinh_nan, (FLOAT_T)NAN, 0, 0),
        TEST(asinh_0, (FLOAT_T)0.0, 0, 0),
        TEST(asinh_neg0, (FLOAT_T)-0.0, 0, 0),
        TEST(asinh_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(asinh_minf, (FLOAT_T)-INFINITY, 0, 0),

        TEST(atan2_nanx, (FLOAT_T)NAN, 0, 0),
        TEST(atan2_nany, (FLOAT_T)NAN, 0, 0),
        TEST(atan2_tiny, (FLOAT_T)0.0, FE_UNDERFLOW|FE_INEXACT, ERANGE),
        TEST(atan2_nottiny, (FLOAT_T)M_PI, 0, 0),

        TEST(atanh_nan, (FLOAT_T)NAN, 0, 0),
        TEST(atanh_1, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
        TEST(atanh_neg1, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
        TEST(atanh_2, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(atanh_neg2, (FLOAT_T)NAN, FE_INVALID, EDOM),

        TEST(cbrt_0, (FLOAT_T)0.0, 0, 0),
        TEST(cbrt_neg0, -(FLOAT_T)0.0, 0, 0),
        TEST(cbrt_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(cbrt_neginf, -(FLOAT_T)INFINITY, 0, 0),
        TEST(cbrt_nan, (FLOAT_T)NAN, 0, 0),

        TEST(cos_inf, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(cos_nan, (FLOAT_T)NAN, 0, 0),
        TEST(cos_0, (FLOAT_T)1.0, 0, 0),

        TEST(cosh_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
        TEST(cosh_negbig, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),

	TEST(drem_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(drem_nan_1, (FLOAT_T)NAN, 0, 0),
	TEST(drem_1_nan, (FLOAT_T)NAN, 0, 0),
        TEST(drem_inf_2, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(drem_inf_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(drem_2_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(drem_1_2, (FLOAT_T)1.0, 0, 0),
        TEST(drem_neg1_2, -(FLOAT_T)1.0, 0, 0),

        TEST(erf_nan, (FLOAT_T) NAN, 0, 0),
        TEST(erf_0, (FLOAT_T) 0, 0, 0),
        TEST(erf_neg0, -(FLOAT_T) 0, 0, 0),
        TEST(erf_inf, (FLOAT_T) 1.0, 0, 0),
        TEST(erf_neginf, -(FLOAT_T) 1.0, 0, 0),
        TEST(erf_small, (FLOAT_T) 2.0 * (FLOAT_T) SMALL / (FLOAT_T) 1.772453850905516027298167, FE_UNDERFLOW, 0),

	TEST(exp_nan, (FLOAT_T)NAN, 0, 0),
	TEST(exp_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(exp_neginf, (FLOAT_T)0.0, 0, 0),
	TEST(exp_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(exp_negbig, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),

	TEST(exp2_nan, (FLOAT_T)NAN, 0, 0),
	TEST(exp2_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(exp2_neginf, (FLOAT_T)0.0, 0, 0),
	TEST(exp2_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(exp2_negbig, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),

	TEST(exp10_nan, (FLOAT_T)NAN, 0, 0),
	TEST(exp10_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(exp10_neginf, (FLOAT_T)0.0, 0, 0),
	TEST(exp10_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(exp10_negbig, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),

	TEST(expm1_nan, (FLOAT_T)NAN, 0, 0),
	TEST(expm1_0, (FLOAT_T)0.0, 0, 0),
	TEST(expm1_neg0, -(FLOAT_T)0.0, 0, 0),
	TEST(expm1_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(expm1_neginf, -(FLOAT_T)1.0, 0, 0),
	TEST(expm1_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(expm1_negbig, -(FLOAT_T)1.0, FE_INEXACT, 0),

        TEST(fabs_nan, (FLOAT_T)NAN, 0, 0),
        TEST(fabs_0, (FLOAT_T)0.0, 0, 0),
        TEST(fabs_neg0, (FLOAT_T)0.0, 0, 0),
        TEST(fabs_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(fabs_neginf, (FLOAT_T)INFINITY, 0, 0),

        TEST(fdim_nan_1, (FLOAT_T)NAN, 0, 0),
        TEST(fdim_1_nan, (FLOAT_T)NAN, 0, 0),
        TEST(fdim_inf_1, (FLOAT_T)INFINITY, 0, 0),
        TEST(fdim_1_inf, (FLOAT_T)0.0, 0, 0),
        TEST(fdim_big_negbig, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),

        TEST(floor_1, (FLOAT_T)1.0, 0, 0),
        TEST(floor_0, (FLOAT_T)0.0, 0, 0),
        TEST(floor_neg0, -(FLOAT_T)0.0, 0, 0),
        TEST(floor_nan, (FLOAT_T)NAN, 0, 0),
        TEST(floor_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(floor_neginf, -(FLOAT_T)INFINITY, 0, 0),

        TEST(fma_big_big_1, (FLOAT_T)INFINITY, FE_OVERFLOW, 0),
        TEST(fma_big_negbig_1, -(FLOAT_T)INFINITY, FE_OVERFLOW, 0),
        TEST(fma_small_small_1, (FLOAT_T)1.0, FE_INEXACT, 0),
        TEST(fma_small_negsmall_1, (FLOAT_T)1.0, FE_INEXACT, 0),
        TEST(fma_small_small_0, (FLOAT_T)0.0, FE_UNDERFLOW, 0),
        TEST(fma_small_negsmall_0, -(FLOAT_T)0.0, FE_UNDERFLOW, 0),
        TEST(fma_nan_1_1, (FLOAT_T)NAN, 0, 0),
        TEST(fma_1_nan_1, (FLOAT_T)NAN, 0, 0),
        TEST(fma_1_1_nan, (FLOAT_T)NAN, 0, 0),
        TEST(fma_inf_1_neginf, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_1_inf_neginf, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_neginf_1_inf, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_1_neginf_inf, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_inf_0_1, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_0_inf_1, (FLOAT_T)NAN, FE_INVALID, 0),
#ifdef __PICOLIBC__
        /* Linux says these will set FE_INVALID, POSIX says optional, glibc does not set exception */
        TEST(fma_inf_0_nan, (FLOAT_T)NAN, FE_INVALID, 0),
        TEST(fma_0_inf_nan, (FLOAT_T)NAN, FE_INVALID, 0),
#endif

        TEST(fmax_nan_nan, (FLOAT_T)NAN, 0, 0),
        TEST(fmax_nan_1, (FLOAT_T)1.0, 0, 0),
        TEST(fmax_1_nan, (FLOAT_T)1.0, 0, 0),

        TEST(fmin_nan_nan, (FLOAT_T)NAN, 0, 0),
        TEST(fmin_nan_1, (FLOAT_T)1.0, 0, 0),
        TEST(fmin_1_nan, (FLOAT_T)1.0, 0, 0),

        TEST(fmod_nan_1, (FLOAT_T)NAN, 0, 0),
        TEST(fmod_1_nan, (FLOAT_T)NAN, 0, 0),
        TEST(fmod_inf_1, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(fmod_neginf_1, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(fmod_1_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(fmod_0_1, (FLOAT_T)0.0, 0, 0),
        TEST(fmod_neg0_1, -(FLOAT_T)0.0, 0, 0),

        TEST(gamma_nan, (FLOAT_T)NAN, 0, 0),
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

	TEST(hypot_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
        TEST(hypot_1_nan, (FLOAT_T)NAN, 0, 0),
        TEST(hypot_nan_1, (FLOAT_T)NAN, 0, 0),
        TEST(hypot_inf_nan, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_neginf_nan, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_nan_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_nan_neginf, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_1_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_1_neginf, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_inf_1, (FLOAT_T)INFINITY, 0, 0),
        TEST(hypot_neginf_1, (FLOAT_T)INFINITY, 0, 0),

        TEST(j0_inf, 0, 0, 0),
        TEST(j0_nan, (FLOAT_T)NAN, 0, 0),

        TEST(j1_inf, 0, 0, 0),
        TEST(j1_nan, (FLOAT_T)NAN, 0, 0),

        TEST(jn_inf, 0, 0, 0),
        TEST(jn_nan, (FLOAT_T)NAN, 0, 0),

        TEST(ldexp_1_0, (FLOAT_T)1.0, 0, 0),
        TEST(ldexp_nan_0, (FLOAT_T)NAN, 0, 0),
        TEST(ldexp_inf_0, (FLOAT_T)INFINITY, 0, 0),
        TEST(ldexp_neginf_0, -(FLOAT_T)INFINITY, 0, 0),
        TEST(ldexp_1_negbig, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),
        TEST(ldexp_1_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),

        TEST(lgamma_nan, (FLOAT_T)NAN, 0, 0),
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

        TEST(lgamma_r_nan, (FLOAT_T)NAN, 0, 0),
        TEST(lgamma_r_1, (FLOAT_T) 0.0, 0, 0),
        TEST(lgamma_r_2, (FLOAT_T) 0.0, 0, 0),
	TEST(lgamma_r_0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_neg0, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_neg1, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_neg2, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(lgamma_r_negbig, (FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(lgamma_r_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(lgamma_r_neginf, (FLOAT_T)INFINITY, 0, 0),

	TEST(log_nan, (FLOAT_T)NAN, 0, 0),
	TEST(log_1, (FLOAT_T)0, 0, 0),
	TEST(log_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(log_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(log_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),

	TEST(log10_nan, (FLOAT_T)NAN, 0, 0),
	TEST(log10_1, (FLOAT_T)0, 0, 0),
	TEST(log10_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(log10_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log10_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(log10_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),

	TEST(log1p_nan, (FLOAT_T)NAN, 0, 0),
	TEST(log1p_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(log1p_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(log1p_neg1, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log1p_neg2, (FLOAT_T)NAN, FE_INVALID, EDOM),

	TEST(log2_nan, (FLOAT_T)NAN, 0, 0),
	TEST(log2_1, (FLOAT_T)0, 0, 0),
	TEST(log2_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(log2_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(log2_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(log2_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),

        TEST(logb_nan, (FLOAT_T)NAN, 0, 0),
        TEST(logb_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, 0),
        TEST(logb_neg0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, 0),
        TEST(logb_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(logb_neginf, (FLOAT_T)INFINITY, 0, 0),

	TEST(pow_neg_half, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(pow_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow_negbig, (FLOAT_T)-INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow_tiny, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),
        TEST(pow_1_1, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_2, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_neg2, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_inf, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_neginf, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_nan, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_1_neg0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_0_0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_0_neg0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_inf_0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_inf_neg0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_nan_0, (FLOAT_T)1.0, 0, 0),
        TEST(pow_nan_neg0, (FLOAT_T)1.0, 0, 0),
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

	TEST(pow10_nan, (FLOAT_T)NAN, 0, 0),
	TEST(pow10_inf, (FLOAT_T)INFINITY, 0, 0),
	TEST(pow10_neginf, (FLOAT_T)0.0, 0, 0),
	TEST(pow10_big, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(pow10_negbig, (FLOAT_T)0.0, FE_UNDERFLOW, ERANGE),

	TEST(remainder_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
	TEST(remainder_nan_1, (FLOAT_T)NAN, 0, 0),
	TEST(remainder_1_nan, (FLOAT_T)NAN, 0, 0),
        TEST(remainder_inf_2, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(remainder_inf_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(remainder_2_0, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(remainder_1_2, (FLOAT_T)1.0, 0, 0),
        TEST(remainder_neg1_2, -(FLOAT_T)1.0, 0, 0),

        TEST(scalb_1_1, (FLOAT_T)2.0, 0, 0),
        TEST(scalb_1_half, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(scalb_nan_1, (FLOAT_T)NAN, 0, 0),
        TEST(scalb_1_nan, (FLOAT_T)NAN, 0, 0),
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

        TEST(sin_inf, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(sin_nan, (FLOAT_T)NAN, 0, 0),
        TEST(sin_pio2, (FLOAT_T)1.0, FE_INEXACT, 0),
        TEST(sin_small, (FLOAT_T)SMALL, FE_INEXACT, 0),
        TEST(sin_0, (FLOAT_T)0.0, 0, 0),

        TEST(sincos, (FLOAT_T)1.0, 0, 0),
        TEST(sincos_inf, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(sincos_nan, (FLOAT_T)NAN, 0, 0),

        TEST(sinh_nan, (FLOAT_T)NAN, 0, 0),
        TEST(sinh_0, (FLOAT_T)0.0, 0, 0),
        TEST(sinh_neg0, (FLOAT_T)-0.0, 0, 0),
        TEST(sinh_inf, (FLOAT_T)INFINITY, 0, 0),
        TEST(sinh_neginf, (FLOAT_T)-INFINITY, 0, 0),

	TEST(sqrt_nan, (FLOAT_T)NAN, 0, 0),
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

        TEST(tan_nan, (FLOAT_T)NAN, 0, 0),
        TEST(tan_inf, (FLOAT_T)NAN, FE_INVALID, EDOM),
        TEST(tan_neginf, (FLOAT_T)NAN, FE_INVALID, EDOM),
#if 0
        /* alas, we aren't close enough to π/2 to overflow */
        TEST(tan_pio2, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
#endif

        TEST(tanh_nan, (FLOAT_T)NAN, 0, 0),
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
#if !defined(TEST_FLOAT) || defined(__PICOLIBC__)
	/* glibc has a bug with this test using float */
	TEST(tgamma_small, (FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
	TEST(tgamma_negsmall, -(FLOAT_T)INFINITY, FE_OVERFLOW, ERANGE),
#endif

	TEST(y0_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
        TEST(y0_nan, (FLOAT_T)NAN, 0, 0),
	TEST(y0_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),

	TEST(y1_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(y1_nan, (FLOAT_T)NAN, 0, 0),
	TEST(y1_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),

	TEST(yn_0, -(FLOAT_T)INFINITY, FE_DIVBYZERO, ERANGE),
	TEST(yn_nan, (FLOAT_T)NAN, 0, 0),
	TEST(yn_neg, (FLOAT_T)NAN, FE_INVALID, EDOM),

	{ 0 },
};

struct {
        long    (*func)(void);
	char	*name;
	long	value;
	int	except;
	int	errno_expect;
} makemathname(itests)[] = {
        TEST(ilogb_0, FP_ILOGB0, FE_INVALID, EDOM),
        TEST(ilogb_nan, FP_ILOGBNAN, FE_INVALID, EDOM),
        TEST(ilogb_inf, INT_MAX, FE_INVALID, EDOM),
        TEST(ilogb_neginf, INT_MAX, FE_INVALID, EDOM),

        TEST(lrint_nan, 0, FE_INVALID, 0),
        TEST(lrint_inf, 0, FE_INVALID, 0),
        TEST(lrint_neginf, 0, FE_INVALID, 0),
        TEST(lrint_big, 0, FE_INVALID, 0),
        TEST(lrint_negbig, 0, FE_INVALID, 0),

        TEST(lround_nan, 0, FE_INVALID, 0),
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
		except = fetestexcept(FE_ALL_EXCEPT);
		if ((isinf(v) && isinf(makemathname(tests)[t].value) && ((v > 0) != (makemathname(tests)[t].value > 0))) ||
		    (v != makemathname(tests)[t].value && (!isnanf(v) || !isnanf(makemathname(tests)[t].value))))
		{
                        PRINT;
			printf("\tbad value got %0.17g expect %0.17g\n", (double) v, (double) makemathname(tests)[t].value);
			++result;
		}
		if (math_errhandling & EXCEPTION_TEST) {
			if ((except & makemathname(tests)[t].except) != makemathname(tests)[t].except) {
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
				printf("\terrno supported but %s returns %d (%s)\n", makemathname(tests)[t].name, err, strerror(err));
				++result;
			}
		} else {
			if (err != 0) {
                                PRINT;
				printf("\terrno not supported but %s returns %d (%s)\n", makemathname(tests)[t].name, err, strerror(err));
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
		except = fetestexcept(FE_ALL_EXCEPT);
		if (iv != makemathname(itests)[t].value)
		{
                        IPRINT;
			printf("\tbad value got %ld expect %ld\n", iv, makemathname(itests)[t].value);
			++result;
		}
		if (math_errhandling & EXCEPTION_TEST) {
			if ((except & makemathname(itests)[t].except) != makemathname(itests)[t].except) {
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
