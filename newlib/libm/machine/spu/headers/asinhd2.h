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
#ifndef _ASINHD2_H_
#define _ASINHD2_H_	1

#include <spu_intrinsics.h>

#include "logd2.h"
#include "sqrtd2.h"

/*
 * FUNCTION
 *  vector double _asinhd2(vector double x)
 *
 * DESCRIPTION
 *  The asinhd2 function returns a vector containing the hyperbolic
 *  arcsines of the corresponding elements of the input vector.
 *
 *  We are using the formula:
 *    asinh = ln(|x| + sqrt(x^2 + 1))
 *  and the anti-symmetry of asinh.
 *
 *  For x near zero, we use the Taylor series:
 *
 *                infinity
 *                 ------
 *                  -   '  P  (0)
 *                   -      k-1    k
 *    asinh x =       -    -----  x
 *                   -       k
 *                  -   ,
 *                 ------
 *                 k = 1
 *
 *  Special Cases:
 *    asinh(+0)        returns +0
 *    asinh(-0)        returns -0
 *    asinh(+infinity) returns +infinity
 *    asinh(-infinity) returns -infinity
 *    asinh(NaN)       returns NaN
 *
 */

/*
 * Maclaurin Series Coefficients 
 * for x near 0.
 */
#define SDM_ASINHD2_MAC01     1.000000000000000000000000000000000000000000E0
#define SDM_ASINHD2_MAC03    -1.666666666666666666666666666666666666666667E-1
#define SDM_ASINHD2_MAC05     7.500000000000000000000000000000000000000000E-2
#define SDM_ASINHD2_MAC07    -4.464285714285714285714285714285714285714286E-2
#define SDM_ASINHD2_MAC09     3.038194444444444444444444444444444444444444E-2
#define SDM_ASINHD2_MAC11    -2.237215909090909090909090909090909090909091E-2
#define SDM_ASINHD2_MAC13     1.735276442307692307692307692307692307692308E-2
#define SDM_ASINHD2_MAC15    -1.396484375000000000000000000000000000000000E-2
#define SDM_ASINHD2_MAC17     1.155180089613970588235294117647058823529412E-2


static __inline vector double _asinhd2(vector double x)
{
    vec_double2 sign_mask = spu_splats(-0.0);
    vec_double2 oned      = spu_splats(1.0);
    vec_uchar16 dup_even  = ((vec_uchar16) { 0,1,2,3, 0,1,2,3, 8,9,10,11, 8,9,10,11 });
    vec_uint4   infminus1 = spu_splats(0x7FEFFFFFU);
    vec_uint4   isinfnan;
    vec_double2 xabs, xsqu;
    vec_uint4   xabshigh;
    vec_float4  switch_approx = spu_splats(0.165f);  /* Where we switch from maclaurin to formula */
    vec_uint4   use_form;
    vec_float4  xf;
    vec_double2 result, fresult, mresult;


    xabs = spu_andc(x, sign_mask);
    xsqu = spu_mul(x, x);

    xf = spu_roundtf(xabs);
    xf = spu_shuffle(xf, xf, dup_even);

    /*
     * Formula:
     *   asinh = ln(|x| + sqrt(x^2 + 1))
     */
    fresult = _sqrtd2(spu_add(xsqu, oned));
    fresult = spu_add(xabs, fresult);
    fresult = _logd2(fresult);


    /*
     * Maclaurin Series approximation
     */

    mresult = spu_splats(SDM_ASINHD2_MAC17);
    mresult = spu_madd(xsqu, mresult, spu_splats(SDM_ASINHD2_MAC15));
    mresult = spu_madd(xsqu, mresult, spu_splats(SDM_ASINHD2_MAC13));
    mresult = spu_madd(xsqu, mresult, spu_splats(SDM_ASINHD2_MAC11));
    mresult = spu_madd(xsqu, mresult, spu_splats(SDM_ASINHD2_MAC09));
    mresult = spu_madd(xsqu, mresult, spu_splats(SDM_ASINHD2_MAC07));
    mresult = spu_madd(xsqu, mresult, spu_splats(SDM_ASINHD2_MAC05));
    mresult = spu_madd(xsqu, mresult, spu_splats(SDM_ASINHD2_MAC03));
    mresult = spu_madd(xsqu, mresult, spu_splats(SDM_ASINHD2_MAC01));
    mresult = spu_mul(xabs, mresult);


    /*
     * Choose between series and formula
     */
    use_form = spu_cmpgt(xf, switch_approx);
    result = spu_sel(mresult, fresult, (vec_ullong2)use_form);


    /* Special Cases */

    /* Infinity and NaN */
    xabshigh = (vec_uint4)spu_shuffle(xabs, xabs, dup_even);
    isinfnan = spu_cmpgt(xabshigh, infminus1);
    result = spu_sel(result, x, (vec_ullong2)isinfnan);


    /* Restore sign - asinh is an anti-symmetric */
    result = spu_sel(result, x, (vec_ullong2)sign_mask);

    return result;
}

#endif /* _ASINHD2_H_ */
#endif /* __SPU__ */
