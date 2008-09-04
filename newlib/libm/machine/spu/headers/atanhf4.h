/* --------------------------------------------------------------  */
/* (C)Copyright 2007,2008,                                         */
/* International Business Machines Corporation                     */
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
#ifndef _ATANHF4_H_
#define _ATANHF4_H_	1

#include <spu_intrinsics.h>
#include <math.h>
#include "logf4.h"

/*
 * FUNCTION
 *  vector float _atanhf4(vector float x)
 *
 * DESCRIPTION
 *  The atanhf4 function returns a vector containing the hyperbolic
 *  arctangents of the corresponding elements of the input vector.
 *
 *  We are using the formula:
 *    atanh x = 1/2 * ln((1 + x)/(1 - x)) = 1/2 * [ln(1+x) - ln(1-x)]
 *  and the anti-symmetry of atanh.
 *
 *  For x near 0, we use the Taylor series:
 *    atanh x = x + x^3/3 + x^5/5 + x^7/7 + x^9/9 + ...
 *
 *  Special Cases:
 *  - atanh(1)  =  HUGE_VALF
 *  - atanh(-1) = -HUGE_VALF
 *	- The result is undefined for x outside of the domain [-1,1].
 *
 */

/*
 * Maclaurin Series Coefficients 
 * for x near 0.
 */
#define SDM_SP_ATANH_MAC01 1.000000000000000000000000000000E0
#define SDM_SP_ATANH_MAC03 3.333333333333333333333333333333E-1
#define SDM_SP_ATANH_MAC05 2.000000000000000000000000000000E-1
#define SDM_SP_ATANH_MAC07 1.428571428571428571428571428571E-1
#if 0
#define SDM_SP_ATANH_MAC09 1.111111111111111111111111111111E-1
#define SDM_SP_ATANH_MAC11 9.090909090909090909090909090909E-2
#define SDM_SP_ATANH_MAC13 7.692307692307692307692307692308E-2
#define SDM_SP_ATANH_MAC15 6.666666666666666666666666666667E-2
#endif


static __inline vector float _atanhf4(vector float x)
{
    vec_uint4  one = spu_splats(1u);
    vec_float4 sign_mask = spu_splats(-0.0f);
    vec_float4 onef      = spu_splats(1.0f);
    vec_float4 onehalff  = spu_splats(0.5f);
    vec_float4 huge      = spu_splats(HUGE_VALF);
    vec_float4 result, fresult, mresult;;
    vec_float4 xabs, xsqu;
    /* Where we switch from maclaurin to formula */
    vec_float4  switch_approx = spu_splats(0.165f);
    vec_uint4   use_form;

    xabs = spu_andc(x, sign_mask);
    xsqu = spu_mul(x, x);

    /*
     * Formula:
     *   atanh = 1/2 * ln((1 + x)/(1 - x)) = 1/2 * [ln(1+x) - ln(1-x)]
     */
    fresult = spu_sub(_logf4(spu_add(onef, xabs)), _logf4(spu_sub(onef, xabs)));
    fresult = spu_mul(fresult, onehalff);


    /*
     * Taylor Series
     */
    mresult = spu_madd(xsqu, spu_splats((float)SDM_SP_ATANH_MAC07), spu_splats((float)SDM_SP_ATANH_MAC05));
    mresult = spu_madd(xsqu, mresult, spu_splats((float)SDM_SP_ATANH_MAC03));
    mresult = spu_madd(xsqu, mresult, spu_splats((float)SDM_SP_ATANH_MAC01));
    mresult = spu_mul(xabs, mresult);

    /*
     * Choose between series and formula
     */
    use_form = spu_cmpgt(xabs, switch_approx);
    result = spu_sel(mresult, fresult, use_form);

    /*
     * Correct for accumulated truncation error. Currently reduces rms of
     * absolute error by about 50%
     */
    result = (vec_float4)spu_add((vec_uint4)result, spu_and(one, spu_cmpgt(xabs, spu_splats(0.0f))));
    result = (vec_float4)spu_add((vec_uint4)result, spu_and(one, spu_cmpgt(xabs, spu_splats(0.25f))));

    /*
     * Check Boundary Conditions
     */
    result = spu_sel(result, huge, spu_cmpeq(xabs, onef));

    /*
     * Spec says |x| > 1, result is undefined, so no additional
     * boundary checks needed.
     */

    /* Preserve sign - atanh is anti-symmetric */
    result = spu_sel(result, x, (vec_uint4)sign_mask);

    return result;
}

#endif /* _ATANHF4_H_ */
#endif /* __SPU__ */
