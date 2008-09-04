/* --------------------------------------------------------------  */
/* (C)Copyright 2006,2007,                                         */
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
/* Redistributions of source code must retain the above copyright  */
/* notice, this list of conditions and the following disclaimer.   */
/*                                                                 */
/* Redistributions in binary form must reproduce the above         */
/* copyright notice, this list of conditions and the following     */
/* disclaimer in the documentation and/or other materials          */
/* provided with the distribution.                                 */
/*                                                                 */
/* Neither the name of IBM Corporation nor the names of its        */
/* contributors may be used to endorse or promote products         */
/* derived from this software without specific prior written       */
/* permission.                                                     */
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
#ifndef _ATANHD2_H_
#define _ATANHD2_H_	1

#include <spu_intrinsics.h>
#include "logd2.h"

/*
 * FUNCTION
 *  vector double _atanhd2(vector double x)
 *
 * DESCRIPTION
 *  The atanhd2 function returns a vector containing the hyperbolic
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
 *  - atanh(1)  =  Infinity
 *  - atanh(-1) = -Infinity
 *  - atanh(x) for |x| > 1 = Undefined
 *
 */

/*
 * Maclaurin Series Coefficients 
 * for x near 0.
 */
#define SMD_DP_ATANH_MAC01 1.000000000000000000000000000000E0
#define SMD_DP_ATANH_MAC03 3.333333333333333333333333333333E-1
#define SMD_DP_ATANH_MAC05 2.000000000000000000000000000000E-1
#define SMD_DP_ATANH_MAC07 1.428571428571428571428571428571E-1
#define SMD_DP_ATANH_MAC09 1.111111111111111111111111111111E-1
#define SMD_DP_ATANH_MAC11 9.090909090909090909090909090909E-2
#define SMD_DP_ATANH_MAC13 7.692307692307692307692307692308E-2
#define SMD_DP_ATANH_MAC15 6.666666666666666666666666666667E-2
#define SMD_DP_ATANH_MAC17 5.882352941176470588235294117647E-2
#if 0
#define SMD_DP_ATANH_MAC19 5.263157894736842105263157894737E-2
#define SMD_DP_ATANH_MAC21 4.761904761904761904761904761905E-2
#define SMD_DP_ATANH_MAC23 4.347826086956521739130434782609E-2
#define SMD_DP_ATANH_MAC25 4.000000000000000000000000000000E-2
#define SMD_DP_ATANH_MAC27 3.703703703703703703703703703704E-2
#define SMD_DP_ATANH_MAC29 3.448275862068965517241379310345E-2
#define SMD_DP_ATANH_MAC31 3.225806451612903225806451612903E-2
#define SMD_DP_ATANH_MAC33 3.030303030303030303030303030303E-2
#define SMD_DP_ATANH_MAC35 2.857142857142857142857142857143E-2
#define SMD_DP_ATANH_MAC37 2.702702702702702702702702702703E-2
#define SMD_DP_ATANH_MAC39 2.564102564102564102564102564103E-2
#endif


static __inline vector double _atanhd2(vector double x)
{
    vec_uchar16 dup_even  = ((vec_uchar16) { 0,1,2,3,  0,1,2,3, 8,9,10,11, 8,9,10,11 });
    vec_double2 sign_mask = spu_splats(-0.0);
    vec_double2 oned      = spu_splats(1.0);
    vec_double2 onehalfd  = spu_splats(0.5);
    vec_double2 xabs, xsqu;
    /* Where we switch from maclaurin to formula */
    vec_float4  switch_approx = spu_splats(0.125f);
    vec_uint4   use_form;
    vec_float4  xf;
    vec_double2 result, fresult, mresult;;

    xabs = spu_andc(x, sign_mask);
    xsqu = spu_mul(x, x);

    xf = spu_roundtf(xabs);
    xf = spu_shuffle(xf, xf, dup_even);

    /*
     * Formula:
     *   atanh = 1/2 * ln((1 + x)/(1 - x)) = 1/2 * [ln(1+x) - ln(1-x)]
     */
    fresult = spu_sub(_logd2(spu_add(oned, xabs)), _logd2(spu_sub(oned, xabs)));
    fresult = spu_mul(fresult, onehalfd);


    /*
     * Taylor Series
     */
    mresult = spu_madd(xsqu, spu_splats(SMD_DP_ATANH_MAC17), spu_splats(SMD_DP_ATANH_MAC15));
    mresult = spu_madd(xsqu, mresult, spu_splats(SMD_DP_ATANH_MAC13));
    mresult = spu_madd(xsqu, mresult, spu_splats(SMD_DP_ATANH_MAC11));
    mresult = spu_madd(xsqu, mresult, spu_splats(SMD_DP_ATANH_MAC09));
    mresult = spu_madd(xsqu, mresult, spu_splats(SMD_DP_ATANH_MAC07));
    mresult = spu_madd(xsqu, mresult, spu_splats(SMD_DP_ATANH_MAC05));
    mresult = spu_madd(xsqu, mresult, spu_splats(SMD_DP_ATANH_MAC03));
    mresult = spu_madd(xsqu, mresult, spu_splats(SMD_DP_ATANH_MAC01));
    mresult = spu_mul(xabs, mresult);


    /*
     * Choose between series and formula
     */
    use_form = spu_cmpgt(xf, switch_approx);
    result = spu_sel(mresult, fresult, (vec_ullong2)use_form);

    /*
     * Spec says results are undefined for |x| > 1, so
     * no boundary tests needed here.
     */

    /* Restore sign - atanh is an anti-symmetric */
    result = spu_sel(result, x, (vec_ullong2)sign_mask);

    return result;
}

#endif /* _ATANHD2_H_ */
#endif /* __SPU__ */
