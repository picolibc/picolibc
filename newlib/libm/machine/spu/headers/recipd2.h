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

#ifndef _RECIPD2_H_
#define _RECIPD2_H_		1

#include <spu_intrinsics.h>


/*
 * FUNCTION
 *	vector double _recipd2(vector double value)
 * 
 * DESCRIPTION
 * 	The _recipd2 function inverts "value" and returns the result. 
 *      Computation is performed using the single precision reciprocal 
 *      estimate and interpolate instructions to produce a 12 accurate 
 *      estimate.
 *
 *	One (1) iteration of a Newton-Raphson is performed to improve 
 *	accuracy to single precision floating point. Two additional double 
 *	precision iterations are  needed to achieve a full double
 *	preicision result.
 *
 *	The Newton-Raphson iteration is of the form:
 *	a)	X[i+1] = X[i] * (2.0 - b*X[i]) 
 *          or
 *      b)	X[i+1] = X[i] + X[i]*(1.0 - X[i]*b)
 * 	where b is the input value to be inverted
 *
 *      The later (b) form improves the accuracy to 99.95% correctly rounded.
 */ 
static __inline vector double _recipd2(vector double value_in)
{
  vec_float4  x0;
  vec_float4  value;
  vec_float4  one   = spu_splats(1.0f);
  vec_double2 one_d = spu_splats(1.0);
  vec_double2 x1, x2, x3;
  vec_double2 scale;
  vec_double2 exp, value_d;
  vec_ullong2 expmask = spu_splats(0x7FF0000000000000ULL);
  vec_ullong2 is0inf;

#ifdef __SPU_EDP__
  vec_ullong2 isdenorm;
  vec_ullong2 expmask_minus1 = spu_splats(0x7FE0000000000000ULL);

  /* Determine special input values. For example, if the input is a denorm, infinity or 0 */

  isdenorm = spu_testsv(value_in, (SPU_SV_POS_DENORM   | SPU_SV_NEG_DENORM));
  is0inf   = spu_testsv(value_in, (SPU_SV_NEG_ZERO     | SPU_SV_POS_ZERO |
				   SPU_SV_NEG_INFINITY | SPU_SV_POS_INFINITY));

  /* Scale the divisor to correct for double precision floating
   * point exponents that are out of single precision range.
   */
  exp = spu_and(value_in, (vec_double2)expmask);
  scale = spu_xor(exp, (vec_double2)spu_sel(expmask, expmask_minus1, isdenorm));
  value_d = spu_mul(value_in, scale);
  value = spu_roundtf(value_d);

  /* Perform reciprocal with 1 single precision and 2 double precision
   * Newton-Raphson iterations.
   */
  x0 = spu_re(value);
  x1 = spu_extend(spu_madd(spu_nmsub(value, x0, one), x0, x0));
  x2 = spu_madd(spu_nmsub(value_d, x1, one_d), x1, x1);
  x3 = spu_madd(spu_nmsub(value_d, x2, one_d), x2, x2);
  x3 = spu_sel(spu_mul(x3, scale), spu_xor(value_in, (vector double)expmask), is0inf);

#else /* !__SPU_EDP__ */

  vec_uint4 isinf, iszero, isdenorm0;
  vec_double2 value_abs;
  vec_double2 sign = spu_splats(-0.0);
  vec_double2 denorm_scale = (vec_double2)spu_splats(0x4330000000000000ULL);
  vec_double2 exp_53 = (vec_double2)spu_splats(0x0350000000000000ULL);
  vec_uchar16 splat_hi = (vec_uchar16){0,1,2,3, 0,1,2,3, 8,9,10,11, 8,9,10,11};
  vec_uchar16 swap = (vec_uchar16){4,5,6,7, 0,1,2,3, 12,13,14,15, 8,9,10,11};

  value_abs = spu_andc(value_in, sign);
  exp = spu_and(value_in, (vec_double2)expmask);

  /* Determine if the input is a special value. These include:
   *  denorm   - then we must coerce it to a normal value. 
   *  zero     - then we must return an infinity
   *  infinity - then we must return a zero.
   */
  isdenorm0 = spu_cmpeq(spu_shuffle((vec_uint4)exp, (vec_uint4)exp, splat_hi), 0);

  isinf  = spu_cmpeq((vec_uint4)value_abs, (vec_uint4)expmask);
  iszero = spu_cmpeq((vec_uint4)value_abs, 0);
  isinf  = spu_and(isinf,  spu_shuffle(isinf, isinf, swap));
  iszero = spu_and(iszero, spu_shuffle(iszero, iszero, swap));
  is0inf = (vec_ullong2)spu_or(isinf, iszero);

  /* If the inputs is a denorm, we must first convert it to a normal number since
   * arithmetic operations on denormals produces 0 on Cell/B.E.
   */
  value_d = spu_sub(spu_or(value_abs, exp_53), exp_53);
  value_d = spu_sel(value_abs, value_d, (vec_ullong2)isdenorm0);

  /* Scale the divisor to correct for double precision floating
   * point exponents that are out of single precision range.
   */
  scale = spu_xor(spu_and(value_d, (vec_double2)expmask), (vec_double2)expmask);
  value_d = spu_mul(value_d, scale);
  value = spu_roundtf(value_d);

  /* Perform reciprocal with 1 single precision and 2 double precision
   * Newton-Raphson iterations. The bias is removed after the single 
   * precision iteration.
   */
  x0 = spu_re(value);
  x1 = spu_extend(spu_madd(spu_nmsub(value, x0, one), x0, x0));
  x2 = spu_madd(spu_nmsub(value_d, x1, one_d), x1, x1);
  x3 = spu_madd(spu_nmsub(value_d, x2, one_d), x2, x2);
  x3 = spu_mul(x3, spu_sel(scale, value_in, (vec_ullong2)sign));
  x3 = spu_sel(x3, spu_mul(x3, denorm_scale), (vec_ullong2)isdenorm0);
  x3 = spu_sel(x3, spu_xor(value_in, (vector double)expmask), is0inf);

#endif /* __SPU_EDP__ */

  return (x3);
}

#endif /* _RECIPD2_H_ */
#endif /* __SPU__ */
