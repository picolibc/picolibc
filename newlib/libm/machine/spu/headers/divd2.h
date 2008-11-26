/* --------------------------------------------------------------  */
/* (C)Copyright 2001,2008,                                         */
/* International Business Machines Corporation,                    */
/* Sony Computer Entertainment, Incorporated,                      */
/* Toshiba Corporation,                                            */
/*                                                                 */
/* All Rights Reserved.                                            */
/*                                                                 */
/* Redistribution and use in source and binary forms, with or      */
/* without modification, are permitted provided that the           */
/* following conditions are met:                                   */
/*                                                                 */
/* - Redistributions of source code must retain the above copyright*/
/*   notice, this list of conditions and the following disclaimer. */
/*                                                                 */
/* - Redistributions in binary form must reproduce the above       */
/*   copyright notice, this list of conditions and the following   */
/*   disclaimer in the documentation and/or other materials        */
/*   provided with the distribution.                               */
/*                                                                 */
/* - Neither the name of IBM Corporation nor the names of its      */
/*   contributors may be used to endorse or promote products       */
/*   derived from this software without specific prior written     */
/*   permission.                                                   */
/*                                                                 */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND          */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,     */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR            */
/* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT    */
/* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;    */
/* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)        */
/* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN       */
/* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR    */
/* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  */
/* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              */
/* --------------------------------------------------------------  */
/* PROLOG END TAG zYx                                              */
#ifdef __SPU__

#ifndef _DIVD2_H_
#define _DIVD2_H_		 1

#include <spu_intrinsics.h>

/*
 * FUNCTION
 * 	vector double _divd2(vector double a, vector double b)
 * 
 * DESCRIPTION
 * 	_divd2 divides the vector dividend a by the vector divisor b and 
 *      returns the resulting vector quotient.  Maximum error about 0.5 ulp 
 *      over entire double range including denorms, compared to true result
 *      in round-to-nearest rounding mode.  Handles Inf or NaN operands and 
 *	results correctly.
 */
static __inline vector double _divd2(vector double a_in, vector double b_in)
{
  /* Variables */
  vec_int4    exp, exp_bias;
  vec_uint4   no_underflow, overflow;
  vec_float4  mant_bf, inv_bf;
  vec_ullong2 exp_a, exp_b;
  vec_ullong2 a_nan, a_zero, a_inf, a_denorm;
  vec_ullong2 b_nan, b_zero, b_inf, b_denorm;
  vec_ullong2 nan;
  vec_double2 a, b;
  vec_double2 mant_a, mant_b, inv_b, q0, q1, q2, mult;

  /* Constants */
  vec_float4  onef = spu_splats(1.0f);
  vec_ullong2 exp_mask = spu_splats(0x7FF0000000000000ULL);
  vec_double2 one = spu_splats(1.0);

#ifdef __SPU_EDP__  
  vec_double2 denorm_scale = (vec_double2)spu_splats(0x4330000000000000ULL);

  /* Identify all possible special values that must be accomodated including:
   * +-0, +-infinity, +-denorm, and NaNs.
   */
  a_nan    = spu_testsv(a_in, (SPU_SV_NAN));
  a_zero   = spu_testsv(a_in, (SPU_SV_NEG_ZERO     | SPU_SV_POS_ZERO));
  a_inf    = spu_testsv(a_in, (SPU_SV_NEG_INFINITY | SPU_SV_POS_INFINITY));
  a_denorm = spu_testsv(a_in, (SPU_SV_NEG_DENORM   | SPU_SV_POS_DENORM));

  b_nan    = spu_testsv(b_in, (SPU_SV_NAN));
  b_zero   = spu_testsv(b_in, (SPU_SV_NEG_ZERO     | SPU_SV_POS_ZERO));
  b_inf    = spu_testsv(b_in, (SPU_SV_NEG_INFINITY | SPU_SV_POS_INFINITY));
  b_denorm = spu_testsv(b_in, (SPU_SV_NEG_DENORM   | SPU_SV_POS_DENORM));

  /* Scale denorm inputs to into normalized numbers by conditionally scaling the 
   * input parameters.
   */
  a = spu_sel(a_in, spu_mul(a_in, denorm_scale), a_denorm);
  b = spu_sel(b_in, spu_mul(b_in, denorm_scale), b_denorm);

#else /* !__SPU_EDP__ */
  vec_uint4   a_exp, b_exp;
  vec_ullong2 a_mant_0, b_mant_0;
  vec_ullong2 a_exp_1s, b_exp_1s;
  vec_ullong2 sign_exp_mask;

  vec_uint4   exp_mask_u32 = spu_splats((unsigned int)0x7FF00000);
  vec_uchar16 splat_hi = (vec_uchar16){0,1,2,3, 0,1,2,3,  8, 9,10,11, 8,9,10,11};
  vec_uchar16 swap_32 = (vec_uchar16){4,5,6,7, 0,1,2,3, 12,13,14,15, 8,9,10,11};
  vec_ullong2 sign_mask = spu_splats(0x8000000000000000ULL);
  vec_double2 exp_53 = (vec_double2)spu_splats(0x0350000000000000ULL);

  sign_exp_mask = spu_or(sign_mask, exp_mask);

  /* Extract the floating point components from each of the operands including
   * exponent and mantissa.
   */
  a_exp = (vec_uint4)spu_and((vec_uint4)a_in, exp_mask_u32);
  a_exp = spu_shuffle(a_exp, a_exp, splat_hi);
  b_exp = (vec_uint4)spu_and((vec_uint4)b_in, exp_mask_u32);
  b_exp = spu_shuffle(b_exp, b_exp, splat_hi);

  a_mant_0 = (vec_ullong2)spu_cmpeq((vec_uint4)spu_andc((vec_ullong2)a_in, sign_exp_mask), 0);
  a_mant_0 = spu_and(a_mant_0, spu_shuffle(a_mant_0, a_mant_0, swap_32));

  b_mant_0 = (vec_ullong2)spu_cmpeq((vec_uint4)spu_andc((vec_ullong2)b_in, sign_exp_mask), 0);
  b_mant_0 = spu_and(b_mant_0, spu_shuffle(b_mant_0, b_mant_0, swap_32));

  a_exp_1s = (vec_ullong2)spu_cmpeq(a_exp, exp_mask_u32);
  b_exp_1s = (vec_ullong2)spu_cmpeq(b_exp, exp_mask_u32);

  /* Identify all possible special values that must be accomodated including:
   * +-denorm, +-0, +-infinity, and NaNs.
   */
  a_denorm = (vec_ullong2)spu_cmpeq(a_exp, 0);		/* really is a_exp_0 */
  a_nan    = spu_andc(a_exp_1s, a_mant_0);
  a_zero   = spu_and (a_denorm, a_mant_0);
  a_inf    = spu_and (a_exp_1s, a_mant_0);

  b_denorm = (vec_ullong2)spu_cmpeq(b_exp, 0);		/* really is b_exp_0 */
  b_nan    = spu_andc(b_exp_1s, b_mant_0);
  b_zero   = spu_and (b_denorm, b_mant_0);
  b_inf    = spu_and (b_exp_1s, b_mant_0);

  /* Scale denorm inputs to into normalized numbers by conditionally scaling the 
   * input parameters.
   */
  a = spu_sub(spu_or(a_in, exp_53), spu_sel(exp_53, a_in, sign_mask));
  a = spu_sel(a_in, a, a_denorm);

  b = spu_sub(spu_or(b_in, exp_53), spu_sel(exp_53, b_in, sign_mask));
  b = spu_sel(b_in, b, b_denorm);

#endif /* __SPU_EDP__ */

  /* Extract the divisor and dividend exponent and force parameters into the signed 
   * range [1.0,2.0) or [-1.0,2.0).
   */
  exp_a = spu_and((vec_ullong2)a, exp_mask);
  exp_b = spu_and((vec_ullong2)b, exp_mask);

  mant_a = spu_sel(a, one, (vec_ullong2)exp_mask);
  mant_b = spu_sel(b, one, (vec_ullong2)exp_mask);
  
  /* Approximate the single reciprocal of b by using
   * the single precision reciprocal estimate followed by one 
   * single precision iteration of Newton-Raphson.
   */
  mant_bf = spu_roundtf(mant_b);
  inv_bf = spu_re(mant_bf);
  inv_bf = spu_madd(spu_nmsub(mant_bf, inv_bf, onef), inv_bf, inv_bf);

  /* Perform 2 more Newton-Raphson iterations in double precision. The
   * result (q1) is in the range (0.5, 2.0).
   */
  inv_b = spu_extend(inv_bf);
  inv_b = spu_madd(spu_nmsub(mant_b, inv_b, one), inv_b, inv_b);
  q0 = spu_mul(mant_a, inv_b);
  q1 = spu_madd(spu_nmsub(mant_b, q0, mant_a), inv_b, q0);


  /* Determine the exponent correction factor that must be applied 
   * to q1 by taking into account the exponent of the normalized inputs
   * and the scale factors that were applied to normalize them.
   */
  exp = spu_rlmaska(spu_sub((vec_int4)exp_a, (vec_int4)exp_b), -20);
  exp = spu_add(exp, (vec_int4)spu_add(spu_and((vec_int4)a_denorm, -0x34), spu_and((vec_int4)b_denorm, 0x34)));
  
  /* Bias the quotient exponent depending on the sign of the exponent correction
   * factor so that a single multiplier will ensure the entire double precision
   * domain (including denorms) can be achieved.
   *
   *    exp 	       bias q1     adjust exp
   *   =====	       ========    ==========
   *   positive         2^+65         -65
   *   negative         2^-64         +64
   */
  exp_bias = spu_xor(spu_rlmaska(exp, -31), 64);


  exp = spu_sub(exp, exp_bias);

  q1 = spu_sel(q1, (vec_double2)spu_add((vec_int4)q1, spu_sl(exp_bias, 20)), exp_mask);

  /* Compute a multiplier (mult) to applied to the quotient (q1) to produce the 
   * expected result. 
   */
  exp = spu_add(exp, 0x3FF);
  no_underflow = spu_cmpgt(exp, 0);
  overflow = spu_cmpgt(exp, 0x7FF);
  exp = spu_and(spu_sl(exp, 20), (vec_int4)no_underflow);
  exp = spu_and(exp, (vec_int4)exp_mask);
  mult = spu_sel((vec_double2)exp, (vec_double2)exp_mask, (vec_ullong2)overflow);

  /* Handle special value conditions. These include:
   *
   * 1) IF either operand is a NaN OR both operands are 0 or INFINITY THEN a NaN 
   *    results.
   * 2) ELSE IF the dividend is an INFINITY OR the divisor is 0 THEN a INFINITY results.
   * 3) ELSE IF the dividend is 0 OR the divisor is INFINITY THEN a 0 results.
   */
  mult = spu_andc(mult, (vec_double2)spu_or(a_zero, b_inf));
  mult = spu_sel(mult, (vec_double2)exp_mask, spu_or(a_inf, b_zero));

  nan = spu_or(a_nan, b_nan);
  nan = spu_or(nan, spu_and(a_zero, b_zero));
  nan = spu_or(nan, spu_and(a_inf, b_inf));

  mult = spu_or(mult, (vec_double2)nan);

  /* Scale the final quotient */

  q2 = spu_mul(q1, mult);

  return (q2);
}

#endif /* _DIVD2_H_ */
#endif /* __SPU__ */
