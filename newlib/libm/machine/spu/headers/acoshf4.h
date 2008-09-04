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
#ifndef _ACOSHF4_H_
#define _ACOSHF4_H_	1

#include <spu_intrinsics.h>
#include "logf4.h"
#include "sqrtf4.h"

/*
 * FUNCTION
 *  vector float _acoshf4(vector float x)
 *
 * DESCRIPTION
 *  The acoshf4 function returns a vector containing the hyperbolic
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
 *	- NaNs and Infinity aren't supported for single-precision on SPU.
 *
 */

/*
 * Taylor Series Coefficients 
 * for x around 1.
 */
#define SDM_ACOSHF4_TAY01  1.00000000000000000000000000000000000E0f  /* 1 / 1                            */
#define SDM_ACOSHF4_TAY02 -8.33333333333333333333333333333333333E-2f /* 1 / 12                           */
#define SDM_ACOSHF4_TAY03  1.87500000000000000000000000000000000E-2f /* 3 / 160                          */
#define SDM_ACOSHF4_TAY04 -5.58035714285714285714285714285714286E-3f /* 5 / 896                          */
#define SDM_ACOSHF4_TAY05  1.89887152777777777777777777777777778E-3f /* 35 / 18432                       */
#define SDM_ACOSHF4_TAY06 -6.99129971590909090909090909090909091E-4f /* 63 / 90112                       */
#define SDM_ACOSHF4_TAY07  2.71136944110576923076923076923076923E-4f /* 231 / 851968                     */
#define SDM_ACOSHF4_TAY08 -1.09100341796875000000000000000000000E-4f /* 143 / 1310720                    */
#define SDM_ACOSHF4_TAY09  4.51242222505457261029411764705882353E-5f /* 6435 / 142606336                 */
#define SDM_ACOSHF4_TAY10 -1.90656436117071854440789473684210526E-5f /* 12155 / 637534208                */
#define SDM_ACOSHF4_TAY11  8.19368731407892136346726190476190476E-6f /* 46189 / 5637144576               */
#define SDM_ACOSHF4_TAY12 -3.57056927421818608823029891304347826E-6f /* 88179 / 24696061952              */
#define SDM_ACOSHF4_TAY13  1.57402595505118370056152343750000000E-6f /* 676039 / 429496729600            */
#define SDM_ACOSHF4_TAY14 -7.00688192241445735648826316550925926E-7f /* 1300075 / 1855425871872          */
#define SDM_ACOSHF4_TAY15  3.14533061665033215078814276333512931E-7f /* 5014575 / 15942918602752         */
#if 0
#define SDM_ACOSHF4_TAY16 -1.42216292935641362301764949675529234E-7f /* 9694845 / 68169720922112         */
#define SDM_ACOSHF4_TAY17  6.47111067761133282064375552264126864E-8f /* 100180065 / 1548112371908608     */
#define SDM_ACOSHF4_TAY18 -2.96094097811711825280716376645224435E-8f /* 116680311 / 3940649673949184     */
#define SDM_ACOSHF4_TAY19  1.36154380562817937676005090612011987E-8f /* 2268783825 / 166633186212708352  */
#endif



static __inline vector float _acoshf4(vector float x)
{
    vec_float4 minus_onef = spu_splats(-1.0f);
    vec_float4 twof       = spu_splats(2.0f);
    vec_float4 largef     = spu_splats(2.5e19f);
    vec_float4 xminus1;
    /* Where we switch from taylor to formula */
    vec_float4  switch_approx = spu_splats(2.0f);
    vec_uint4   use_form;
    vec_float4 result, fresult, mresult;;


    /*
     * Formula:
     *   acosh = ln(x + sqrt(x^2 - 1))
     */
    fresult = _sqrtf4(spu_madd(x, x, minus_onef));
    fresult = spu_add(x, spu_sel(fresult, x, spu_cmpgt(x, largef)));
    fresult = _logf4(fresult);
    fresult = (vec_float4)spu_add((vec_uint4)fresult, spu_splats(2u));

    /*
     * Taylor Series
     */
    xminus1 = spu_add(x, minus_onef);

    mresult = spu_splats(SDM_ACOSHF4_TAY15);
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY14));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY13));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY12));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY11));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY10));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY09));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY08));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY07));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY06));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY05));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY04));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY03));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY02));
    mresult = spu_madd(xminus1, mresult, spu_splats(SDM_ACOSHF4_TAY01));
    
    mresult = spu_mul(mresult, _sqrtf4(spu_mul(xminus1, twof)));
    mresult = (vec_float4)spu_add((vec_uint4)mresult, spu_splats(1u));

    /*
     * Select series or formula
     */
    use_form = spu_cmpgt(x, switch_approx);
    result = spu_sel(mresult, fresult, use_form);


    return result;
}

#endif /* _ACOSHF4_H_ */
#endif /* __SPU__ */
