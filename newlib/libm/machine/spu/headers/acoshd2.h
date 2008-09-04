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
#ifndef _ACOSHD2_H_
#define _ACOSHD2_H_	1

#include <spu_intrinsics.h>
#include "logd2.h"
#include "sqrtd2.h"

/*
 * FUNCTION
 *  vector double _acoshd2(vector double x)
 *
 * DESCRIPTION
 *  The acoshd2 function returns a vector containing the hyperbolic
 *  arccosines of the corresponding elements of the input vector.
 *
 *  We are using the formula:
 *    acosh = ln(x + sqrt(x^2 - 1))
 *
 *  For x near one, we use the Taylor series:
 *
 *                infinity
 *                 ------
 *                  -   '        
 *                   -                 k
 *    acosh x =       -      C  (x - 1)
 *                   -        k
 *                  -   ,
 *                 ------
 *                 k = 0
 *
 *
 *  Special Cases:
 *	- acosh(1)        = +0
 *	- acosh(NaN)      = NaN
 *	- acosh(Infinity) = Infinity
 *	- acosh(x < 1)    = NaN
 *
 */

/*
 * Taylor Series Coefficients 
 * for x around 1.
 */
#define SDM_ACOSHD2_TAY01  1.000000000000000000000000000000000E0  /* 1 / 1                            */
#define SDM_ACOSHD2_TAY02 -8.333333333333333333333333333333333E-2 /* 1 / 12                           */
#define SDM_ACOSHD2_TAY03  1.875000000000000000000000000000000E-2 /* 3 / 160                          */
#define SDM_ACOSHD2_TAY04 -5.580357142857142857142857142857142E-3 /* 5 / 896                          */
#define SDM_ACOSHD2_TAY05  1.898871527777777777777777777777777E-3 /* 35 / 18432                       */
#define SDM_ACOSHD2_TAY06 -6.991299715909090909090909090909090E-4 /* 63 / 90112                       */
#define SDM_ACOSHD2_TAY07  2.711369441105769230769230769230769E-4 /* 231 / 851968                     */
#define SDM_ACOSHD2_TAY08 -1.091003417968750000000000000000000E-4 /* 143 / 1310720                    */
#define SDM_ACOSHD2_TAY09  4.512422225054572610294117647058823E-5 /* 6435 / 142606336                 */
#define SDM_ACOSHD2_TAY10 -1.906564361170718544407894736842105E-5 /* 12155 / 637534208                */
#define SDM_ACOSHD2_TAY11  8.193687314078921363467261904761904E-6 /* 46189 / 5637144576               */
#define SDM_ACOSHD2_TAY12 -3.570569274218186088230298913043478E-6 /* 88179 / 24696061952              */
#define SDM_ACOSHD2_TAY13  1.574025955051183700561523437500000E-6 /* 676039 / 429496729600            */
#define SDM_ACOSHD2_TAY14 -7.006881922414457356488263165509259E-7 /* 1300075 / 1855425871872          */
#define SDM_ACOSHD2_TAY15  3.145330616650332150788142763335129E-7 /* 5014575 / 15942918602752         */

static __inline vector double _acoshd2(vector double x)
{
    vec_uchar16 dup_even  = ((vec_uchar16) { 0,1,2,3,  0,1,2,3, 8,9,10,11, 8,9,10,11 });
    vec_double2 minus_oned = spu_splats(-1.0);
    vec_double2 twod       = spu_splats(2.0);
    /* Where we switch from taylor to formula */
    vec_float4  switch_approx = spu_splats(1.15f);
    vec_double2 result, fresult, mresult;;

    
    vec_double2 xminus1 = spu_add(x, minus_oned);
    vec_float4  xf = spu_roundtf(x);
    xf = spu_shuffle(xf, xf, dup_even);

    vec_ullong2 use_form = (vec_ullong2)spu_cmpgt(xf, switch_approx);

    vec_double2 sqrtargformula = spu_madd(x, x, minus_oned);
    vec_double2 sqrtargtaylor  = spu_mul(xminus1, twod);
    vec_double2 sqrtarg = spu_sel(sqrtargtaylor, sqrtargformula, use_form);

    vec_double2 sqrtresult = _sqrtd2(sqrtarg);

    /*
     * Formula:
     *   acosh = ln(x + sqrt(x^2 - 1))
     */
    fresult = spu_add(x, sqrtresult);
    fresult = _logd2(fresult);

    /*
     * Taylor Series
     */
    mresult = spu_splats(SDM_ACOSHD2_TAY15);
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY14));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY13));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY12));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY11));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY10));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY09));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY08));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY07));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY06));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY05));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY04));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY03));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY02));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHD2_TAY01));


    mresult = spu_mul(mresult, sqrtresult);


    /*
     * Select series or formula
     */
    result = spu_sel(mresult, fresult, use_form);

    return result;
}

#endif /* _ACOSHD2_H_ */
#endif /* __SPU__ */
