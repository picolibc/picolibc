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

#ifndef _LGAMMAF4_H_
#define _LGAMMAF4_H_	1

#include <spu_intrinsics.h>

#include "logf4.h"
#include "divf4.h"
#include "recipf4.h"
#include "truncf4.h"
#include "sinf4.h"


/*
 * FUNCTION
 *	vector float _lgammaf4(vector float x) - Natural Log of Gamma Function
 *
 * DESCRIPTION
 *	_lgammaf4 calculates the natural logarithm of the absolute value of the gamma
 *	function for the corresponding elements of the input vector.
 *
 * C99 Special Cases:
 *	lgamma(0) returns +infinity
 *	lgamma(1) returns +0
 *	lgamma(2) returns +0
 *	lgamma(negative integer) returns +infinity
 *	lgamma(+infinity) returns +infinity
 *	lgamma(-infinity) returns +infinity
 *
 * Other Cases:
 *  lgamma(Nan) returns Nan
 *  lgamma(Denorm) treated as lgamma(0) and returns +infinity
 *
 */

static __inline vector float _lgammaf4(vector float x) 
{
  vec_float4 result;
  vec_float4 halflog2pi = spu_splats(9.189385332046727417803297364056E-1f);
  vec_float4 logpi      = spu_splats(1.1447298858494001741434273513530587116472948129153f);
  vec_float4 inff       = (vec_float4)spu_splats(0x7F800000);
  vec_float4 zerof      = spu_splats(0.0f);
  vec_float4 onef       = spu_splats(1.0f);
  vec_float4 twof       = spu_splats(2.0f);
  vec_float4 sign_maskf = spu_splats(-0.0f);
  vec_float4 pi         = spu_splats(3.14159265358979323846264338328f);


  /*
   * Unfortunately, some of the approximation methods for lgamma require
   * other basic math computations. Get those out of the way now. The
   * compiler seems to good a good job of scheduling this code with
   * the code that follows.
   */
  vec_uint4  gt0      = spu_cmpgt(x, zerof);
  vec_float4 xabs     = spu_andc(x, sign_maskf);
  vec_float4 ln_x     = _logf4(xabs);
  vec_float4 inv_x    = _recipf4(xabs);
  vec_float4 xtrunc   = _truncf4(x);
  vec_float4 inv_xsqu = spu_mul(inv_x, inv_x);
  vec_uint4 isnaninf  = spu_cmpgt((vec_uint4)xabs, 0x7F7FFFFF);
  vec_uint4 ret_zero  = spu_or(spu_cmpeq(x, onef), spu_cmpeq(x, twof));


  /*
   * First thing we do is setup the description of each partition.
   * This consists of:
   * - Start x of partition
   * - Offset (used for evaluating power series expanded around a point)
   * - Truncation adjustment.
   * - Is approx method in region a rational approximation or just a polynomial
   * - The coefficients used in the poly or rational approximation
   */


  /***************************************************************
   * REGION 0: Approximation Near 0 from Above
   *
   * Use Maclaurin Expansion of lgamma()
   *
   * lgamma(z) = -ln(z) - z * EulerMascheroni + Sum[(-1)^n * z^n * Zeta(n)/n]
   */

#define SDM_LGF4_0_START     0.0f
#define SDM_LGF4_0_OFF       0.0f
#define SDM_LGF4_0_TRUNC     2u
#define SDM_LGF4_0_RATIONAL  0x0u

#define SDM_LGF4_0_00   0.0f
#define SDM_LGF4_0_01  -0.5772156649015328606065121f
#define SDM_LGF4_0_02   0.8224670334241132182362076f
#define SDM_LGF4_0_03  -0.4006856343865314284665794f
#define SDM_LGF4_0_04   0.2705808084277845478790009f
#define SDM_LGF4_0_05  -0.2073855510286739852662731f
#define SDM_LGF4_0_06   1.6955717699740818995241965496515E-1f
#define SDM_LGF4_0_07  -1.4404989676884611811997107854997E-1f
#define SDM_LGF4_0_08   1.2550966952474304242233565481358E-1f
#define SDM_LGF4_0_09  -1.1133426586956469049087252991471E-1f
#define SDM_LGF4_0_10   1.0009945751278180853371459589003E-1f
#define SDM_LGF4_0_11  -9.0954017145829042232609298411497E-2f



  /***************************************************************
   * REGION 1: Above 0 and Below 1
   */
#define SDM_LGF4_1_START     0.20f
#define SDM_LGF4_1_OFF       0.0f
#define SDM_LGF4_1_TRUNC     0u
#define SDM_LGF4_1_RATIONAL  0xFFFFFFFFu

/* Numerator */
#define SDM_LGF4_1_06  5.5247592697706124892083167601451981186889952720891079f
#define SDM_LGF4_1_07  188.42248906442882644741346270888237140890625699348872f
#define SDM_LGF4_1_08  730.89115027907050579364152184942040244662318995470771f
#define SDM_LGF4_1_09  -517.93391251349155395618464682404141737699116911423096f
#define SDM_LGF4_1_10  -866.81293419754982917624255525168901081630973644141406f
#define SDM_LGF4_1_11  459.90872804523394478152324135956113729930154636775805f

/* Denominator */
#define SDM_LGF4_1_00   1.0f
#define SDM_LGF4_1_01   62.356015559548850893358835861387218304619374633480009f
#define SDM_LGF4_1_02   553.64875642095755724931612658933597252336243693499682f
#define SDM_LGF4_1_03   997.28805670393557265195865662557219661414263910835386f
#define SDM_LGF4_1_04   257.10520661440946455560646958565998121417179154677712f
#define SDM_LGF4_1_05   -15.398409585547124178878369413880017200739911288666830f



  /***************************************************************
   * REGION 2: Above 0 and Below 1
   */
#define SDM_LGF4_2_START     0.60f
#define SDM_LGF4_2_OFF       0.69f
#define SDM_LGF4_2_TRUNC     1u
#define SDM_LGF4_2_RATIONAL  0x0u

/* This is a power series expanson of LogGamma around 0.69 */
#define SDM_LGF4_2_00   0.27321026793030387025442491383648273204234f
#define SDM_LGF4_2_01  -1.24869016926209356266849815723905575347988f
#define SDM_LGF4_2_02   1.44985879780363867173410158693003578927407f
#define SDM_LGF4_2_03  -1.11686573274718166516744313082147691068190f
#define SDM_LGF4_2_04   1.14079150485439143731395820215710950729505f
#define SDM_LGF4_2_05  -1.29512166953091144888197173527810141620764f
#define SDM_LGF4_2_06   1.55206382120790061136858894716459302629069f
#define SDM_LGF4_2_07  -1.92227237154565289482911310272968704445560f
#define SDM_LGF4_2_08   2.43478939488445894670349784581009987461638f
#define SDM_LGF4_2_09  -3.13512449573283650741385084753752461908870f
#define SDM_LGF4_2_10   4.08851456399492725127969680590409811177590f
#define SDM_LGF4_2_11   5.38629680478093362448042704719642976375265f



  /***************************************************************
   * REGION 3: Around 1
   */
#define SDM_LGF4_3_START     0.74f
#define SDM_LGF4_3_OFF       1.0f
#define SDM_LGF4_3_TRUNC     2u
#define SDM_LGF4_3_RATIONAL  0x0u

#define SDM_LGF4_3_11   -0.90954017145829042232609298411497266951691494159836e-1f
#define SDM_LGF4_3_10    0.10009945751278180853371459589003190170060195315645f
#define SDM_LGF4_3_09   -0.11133426586956469049087252991471245116506731682165f
#define SDM_LGF4_3_08    0.12550966952474304242233565481358155815737009883123f
#define SDM_LGF4_3_07   -0.14404989676884611811997107854997096565712336579503f
#define SDM_LGF4_3_06    0.16955717699740818995241965496515342131696958167214f
#define SDM_LGF4_3_05   -0.20738555102867398526627309729140683361141618390038f
#define SDM_LGF4_3_04    0.27058080842778454787900092413529197569368773797968f
#define SDM_LGF4_3_03   -0.40068563438653142846657938717048333025499543078016f
#define SDM_LGF4_3_02    0.82246703342411321823620758332301259460947495060340f
#define SDM_LGF4_3_01   -0.57721566490153286060651209008240243104215933593992f
#define SDM_LGF4_3_00    0.0f



  /***************************************************************
   * REGION 4:  Above 1 to Below 2
   */

#define SDM_LGF4_4_START     1.25f
#define SDM_LGF4_4_OFF       1.4616321449683623412626595423257213284681962040064f
#define SDM_LGF4_4_TRUNC     1u
#define SDM_LGF4_4_RATIONAL  0x0u

#define SDM_LGF4_4_00   -0.12148629053584960809551455717769158215135617313000f
#define SDM_LGF4_4_01    0.0f
#define SDM_LGF4_4_02    0.48383612272381058521372238085482537020562860838860f
#define SDM_LGF4_4_03   -0.14758772299453070203095509395083641661852764909458f
#define SDM_LGF4_4_04    0.064624940238912752656100346425238557063086033931734f
#define SDM_LGF4_4_05   -0.032788541088481305500850258549331278505894787737970f
#define SDM_LGF4_4_06    0.017970675115210394292863824811126161810628596070981f
#define SDM_LGF4_4_07   -0.010314223036636387275160254800730296612070784399082f
#define SDM_LGF4_4_08    0.0061005360205178884031365656884883648099463048507839f
#define SDM_LGF4_4_09   -0.0036845696083163732546953776004972425913603137160767f
#define SDM_LGF4_4_10    0.00225976482322181046596248251178293952686321035f
#define SDM_LGF4_4_11   -0.00140225144590445083080002880374741201782467331f



  /***************************************************************
   * REGION 5: Around 2
   */

#define SDM_LGF4_5_START     1.50f
#define SDM_LGF4_5_OFF       2.0f
#define SDM_LGF4_5_TRUNC     1u
#define SDM_LGF4_5_RATIONAL  0x0u

#define SDM_LGF4_5_00    0.0f
#define SDM_LGF4_5_01    0.42278433509846713939348790991759756895784066406008f
#define SDM_LGF4_5_02    0.32246703342411321823620758332301259460947495060340f
#define SDM_LGF4_5_03   -0.6735230105319809513324605383714999692166209744683e-1f
#define SDM_LGF4_5_04    0.2058080842778454787900092413529197569368773797968e-1f
#define SDM_LGF4_5_05   -0.738555102867398526627309729140683361141618390038e-2f
#define SDM_LGF4_5_06    0.289051033074152328575298829848675465030291500547e-2f
#define SDM_LGF4_5_07   -0.119275391170326097711393569282810851426622293789e-2f
#define SDM_LGF4_5_08    0.50966952474304242233565481358155815737009883123e-3f
#define SDM_LGF4_5_09   -0.22315475845357937976141880360134005395620571054e-3f
#define SDM_LGF4_5_10    0.9945751278180853371459589003190170060195315645e-4f
#define SDM_LGF4_5_11   -0.44926236738133141700207502406357860782403250745e-4f



  /***************************************************************
   * REGION 6: Above 2 to Below Stirlings
   */

#define SDM_LGF4_6_START     2.48f
#define SDM_LGF4_6_OFF       0.0f
#define SDM_LGF4_6_TRUNC     2u
#define SDM_LGF4_6_RATIONAL  0xFFFFFFFFu

/* Numerator */
#define SDM_LGF4_6_06  2.8952045264375719070927153893062450394256201846894266f
#define SDM_LGF4_6_07  0.9017557380149600532583460408941390566399250566546766f
#define SDM_LGF4_6_08 -5.0120743649109868270726470406381462995568837028633266f
#define SDM_LGF4_6_09  0.5723176665030477945174549923532715487712277062412760f
#define SDM_LGF4_6_10  0.6107282478237180956153912232438073421489100296366786f
#define SDM_LGF4_6_11  0.0312308625200519550078820867041868696010490562277303f

/* Denominator */
#define SDM_LGF4_6_00  1.0f
#define SDM_LGF4_6_01  4.3592151369378598515798083402849838078885877442021500f
#define SDM_LGF4_6_02  2.6245676641191702420707093818412405820501009602499853f
#define SDM_LGF4_6_03  0.3438846837443412565179153619145215759074092780311669f
#define SDM_LGF4_6_04  0.0078092905528158343621764949220712317164193605131159f
#define SDM_LGF4_6_05  -0.000015217018272713076443927141674684568030697337620f



  /***************************************************************
   * REGION 7: Stirlings - Above 6.0
   *
   */

#define SDM_LGF4_7_START     7.80f
#define SDM_LGF4_7_OFF       0.0f
#define SDM_LGF4_7_TRUNC     5u
#define SDM_LGF4_7_RATIONAL  0x0u

#define SDM_LGF4_7_00    8.3333333333333333333333333333333333333333333333333333333333333333333333E-2f
#define SDM_LGF4_7_01   -2.7777777777777777777777777777777777777777777777777777777777777777777778E-3f
#define SDM_LGF4_7_02    7.9365079365079365079365079365079365079365079365079365079365079365079365E-4f
#define SDM_LGF4_7_03   -5.9523809523809523809523809523809523809523809523809523809523809523809524E-4f
#define SDM_LGF4_7_04    8.4175084175084175084175084175084175084175084175084175084175084175084175E-4f
#define SDM_LGF4_7_05   -1.9175269175269175269175269175269175269175269175269175269175269175269175E-3f
#define SDM_LGF4_7_06    6.4102564102564102564102564102564102564102564102564102564102564102564103E-3f
#define SDM_LGF4_7_07   0.0f
#define SDM_LGF4_7_08   0.0f
#define SDM_LGF4_7_09   0.0f
#define SDM_LGF4_7_10   0.0f
#define SDM_LGF4_7_11   0.0f


  /*
   * Now we load the description of each partition.
   */

  /* Start point for each partition */
  vec_float4 r1start = spu_splats(SDM_LGF4_1_START);
  vec_float4 r2start = spu_splats(SDM_LGF4_2_START);
  vec_float4 r3start = spu_splats(SDM_LGF4_3_START);
  vec_float4 r4start = spu_splats(SDM_LGF4_4_START);
  vec_float4 r5start = spu_splats(SDM_LGF4_5_START);
  vec_float4 r6start = spu_splats(SDM_LGF4_6_START);
  vec_float4 r7start = spu_splats(SDM_LGF4_7_START);

  /* X Offset for each partition */
  vec_float4 xoffseta = (vec_float4) {SDM_LGF4_0_OFF, SDM_LGF4_1_OFF, SDM_LGF4_2_OFF, SDM_LGF4_3_OFF};
  vec_float4 xoffsetb = (vec_float4) {SDM_LGF4_4_OFF, SDM_LGF4_5_OFF, SDM_LGF4_6_OFF, SDM_LGF4_7_OFF};

  /* Truncation Correction for each partition */
  vec_uint4 tcorra = (vec_uint4) {SDM_LGF4_0_TRUNC, SDM_LGF4_1_TRUNC, SDM_LGF4_2_TRUNC, SDM_LGF4_3_TRUNC};
  vec_uint4 tcorrb = (vec_uint4) {SDM_LGF4_4_TRUNC, SDM_LGF4_5_TRUNC, SDM_LGF4_6_TRUNC, SDM_LGF4_7_TRUNC};

  /* Is partition a Rational Approximation */
  vec_uint4 israta = (vec_uint4) {SDM_LGF4_0_RATIONAL, SDM_LGF4_1_RATIONAL, SDM_LGF4_2_RATIONAL, SDM_LGF4_3_RATIONAL};
  vec_uint4 isratb = (vec_uint4) {SDM_LGF4_4_RATIONAL, SDM_LGF4_5_RATIONAL, SDM_LGF4_6_RATIONAL, SDM_LGF4_7_RATIONAL};

  /* The polynomial coefficients for all partitions */
  vec_float4 c00a = (vec_float4) {SDM_LGF4_0_00, SDM_LGF4_1_00, SDM_LGF4_2_00, SDM_LGF4_3_00};
  vec_float4 c01a = (vec_float4) {SDM_LGF4_0_01, SDM_LGF4_1_01, SDM_LGF4_2_01, SDM_LGF4_3_01};
  vec_float4 c02a = (vec_float4) {SDM_LGF4_0_02, SDM_LGF4_1_02, SDM_LGF4_2_02, SDM_LGF4_3_02};
  vec_float4 c03a = (vec_float4) {SDM_LGF4_0_03, SDM_LGF4_1_03, SDM_LGF4_2_03, SDM_LGF4_3_03};
  vec_float4 c04a = (vec_float4) {SDM_LGF4_0_04, SDM_LGF4_1_04, SDM_LGF4_2_04, SDM_LGF4_3_04};
  vec_float4 c05a = (vec_float4) {SDM_LGF4_0_05, SDM_LGF4_1_05, SDM_LGF4_2_05, SDM_LGF4_3_05};
  vec_float4 c06a = (vec_float4) {SDM_LGF4_0_06, SDM_LGF4_1_06, SDM_LGF4_2_06, SDM_LGF4_3_06};
  vec_float4 c07a = (vec_float4) {SDM_LGF4_0_07, SDM_LGF4_1_07, SDM_LGF4_2_07, SDM_LGF4_3_07};
  vec_float4 c08a = (vec_float4) {SDM_LGF4_0_08, SDM_LGF4_1_08, SDM_LGF4_2_08, SDM_LGF4_3_08};
  vec_float4 c09a = (vec_float4) {SDM_LGF4_0_09, SDM_LGF4_1_09, SDM_LGF4_2_09, SDM_LGF4_3_09};
  vec_float4 c10a = (vec_float4) {SDM_LGF4_0_10, SDM_LGF4_1_10, SDM_LGF4_2_10, SDM_LGF4_3_10};
  vec_float4 c11a = (vec_float4) {SDM_LGF4_0_11, SDM_LGF4_1_11, SDM_LGF4_2_11, SDM_LGF4_3_11};

  vec_float4 c00b = (vec_float4) {SDM_LGF4_4_00, SDM_LGF4_5_00, SDM_LGF4_6_00, SDM_LGF4_7_00};
  vec_float4 c01b = (vec_float4) {SDM_LGF4_4_01, SDM_LGF4_5_01, SDM_LGF4_6_01, SDM_LGF4_7_01};
  vec_float4 c02b = (vec_float4) {SDM_LGF4_4_02, SDM_LGF4_5_02, SDM_LGF4_6_02, SDM_LGF4_7_02};
  vec_float4 c03b = (vec_float4) {SDM_LGF4_4_03, SDM_LGF4_5_03, SDM_LGF4_6_03, SDM_LGF4_7_03};
  vec_float4 c04b = (vec_float4) {SDM_LGF4_4_04, SDM_LGF4_5_04, SDM_LGF4_6_04, SDM_LGF4_7_04};
  vec_float4 c05b = (vec_float4) {SDM_LGF4_4_05, SDM_LGF4_5_05, SDM_LGF4_6_05, SDM_LGF4_7_05};
  vec_float4 c06b = (vec_float4) {SDM_LGF4_4_06, SDM_LGF4_5_06, SDM_LGF4_6_06, SDM_LGF4_7_06};
  vec_float4 c07b = (vec_float4) {SDM_LGF4_4_07, SDM_LGF4_5_07, SDM_LGF4_6_07, SDM_LGF4_7_07};
  vec_float4 c08b = (vec_float4) {SDM_LGF4_4_08, SDM_LGF4_5_08, SDM_LGF4_6_08, SDM_LGF4_7_08};
  vec_float4 c09b = (vec_float4) {SDM_LGF4_4_09, SDM_LGF4_5_09, SDM_LGF4_6_09, SDM_LGF4_7_09};
  vec_float4 c10b = (vec_float4) {SDM_LGF4_4_10, SDM_LGF4_5_10, SDM_LGF4_6_10, SDM_LGF4_7_10};
  vec_float4 c11b = (vec_float4) {SDM_LGF4_4_11, SDM_LGF4_5_11, SDM_LGF4_6_11, SDM_LGF4_7_11};


  vec_uchar16 shuffle0 = (vec_uchar16) spu_splats(0x00010203);
  vec_uchar16 shuffle1 = (vec_uchar16) spu_splats(0x04050607);
  vec_uchar16 shuffle2 = (vec_uchar16) spu_splats(0x08090A0B);
  vec_uchar16 shuffle3 = (vec_uchar16) spu_splats(0x0C0D0E0F);
  vec_uchar16 shuffle4 = (vec_uchar16) spu_splats(0x10111213);
  vec_uchar16 shuffle5 = (vec_uchar16) spu_splats(0x14151617);
  vec_uchar16 shuffle6 = (vec_uchar16) spu_splats(0x18191A1B);
  vec_uchar16 shuffle7 = (vec_uchar16) spu_splats(0x1C1D1E1F);


  /*
   * Determine the shuffle pattern based on which partition
   * each element of x is in.
   */

  vec_uchar16 gt_r1start = (vec_uchar16)spu_cmpgt(xabs, r1start);
  vec_uchar16 gt_r2start = (vec_uchar16)spu_cmpgt(xabs, r2start);
  vec_uchar16 gt_r3start = (vec_uchar16)spu_cmpgt(xabs, r3start);
  vec_uchar16 gt_r4start = (vec_uchar16)spu_cmpgt(xabs, r4start);
  vec_uchar16 gt_r5start = (vec_uchar16)spu_cmpgt(xabs, r5start);
  vec_uchar16 gt_r6start = (vec_uchar16)spu_cmpgt(xabs, r6start);
  vec_uchar16 gt_r7start = (vec_uchar16)spu_cmpgt(xabs, r7start);

  vec_uchar16 shufflepattern;
  shufflepattern = spu_sel(shuffle0, shuffle1, gt_r1start);
  shufflepattern = spu_sel(shufflepattern, shuffle2, gt_r2start);
  shufflepattern = spu_sel(shufflepattern, shuffle3, gt_r3start);
  shufflepattern = spu_sel(shufflepattern, shuffle4, gt_r4start);
  shufflepattern = spu_sel(shufflepattern, shuffle5, gt_r5start);
  shufflepattern = spu_sel(shufflepattern, shuffle6, gt_r6start);
  shufflepattern = spu_sel(shufflepattern, shuffle7, gt_r7start);



  /* Use the shuffle pattern to select the coefficients */

  vec_float4 coeff_00 = spu_shuffle(c00a, c00b, shufflepattern);
  vec_float4 coeff_01 = spu_shuffle(c01a, c01b, shufflepattern);
  vec_float4 coeff_02 = spu_shuffle(c02a, c02b, shufflepattern);
  vec_float4 coeff_03 = spu_shuffle(c03a, c03b, shufflepattern);
  vec_float4 coeff_04 = spu_shuffle(c04a, c04b, shufflepattern);
  vec_float4 coeff_06 = spu_shuffle(c06a, c06b, shufflepattern);
  vec_float4 coeff_07 = spu_shuffle(c07a, c07b, shufflepattern);
  vec_float4 coeff_05 = spu_shuffle(c05a, c05b, shufflepattern);
  vec_float4 coeff_08 = spu_shuffle(c08a, c08b, shufflepattern);
  vec_float4 coeff_09 = spu_shuffle(c09a, c09b, shufflepattern);
  vec_float4 coeff_10 = spu_shuffle(c10a, c10b, shufflepattern);
  vec_float4 coeff_11 = spu_shuffle(c11a, c11b, shufflepattern);

  vec_float4 xoffset     = spu_shuffle(xoffseta, xoffsetb, shufflepattern);
  vec_uint4  tcorrection = spu_shuffle(tcorra,   tcorrb,   shufflepattern);
  vec_uint4  isrational  = spu_shuffle(israta,   isratb,   shufflepattern);

  /*
   * We've completed the coeff. setup. Now we actually do the
   * approximation below.
   */

  /* Adjust x value here (for approximations about a point) */
  vec_float4 xappr = spu_sub(xabs, xoffset);

  /* If in Stirling partition, do some setup before the madds */
  xappr = spu_sel(xappr, inv_xsqu, (vector unsigned int)gt_r7start);



  /* Now we do the multiplies - either a big polynomial or
   * a rational approximation. Use Horner's method.
   */
  result = coeff_11;
  result = spu_madd(xappr, result, coeff_10);
  result = spu_madd(xappr, result, coeff_09);
  result = spu_madd(xappr, result, coeff_08);
  result = spu_madd(xappr, result, coeff_07);
  result = spu_madd(xappr, result, coeff_06);

  /* For rational approximations, we save numerator. */
  vec_float4 resultn = result;

  /* For rational appr,, reset result for calculation of denominator. */
  result = spu_sel(result, spu_splats(0.0f), isrational);

  result = spu_madd(xappr, result, coeff_05);
  result = spu_madd(xappr, result, coeff_04);
  result = spu_madd(xappr, result, coeff_03);
  result = spu_madd(xappr, result, coeff_02);
  result = spu_madd(xappr, result, coeff_01);
  result = spu_madd(xappr, result, coeff_00);

  /* Select either the polynomial or rational result */
  result = spu_sel(result, _divf4(resultn, result), isrational);

  /*
   * Now we have to do a bit of additional calculations for
   * partitions that weren't simple polynomial or rational
   * approximations.
   */

  /* Finish the Near 0 formula */
  result = spu_sel(spu_sub(result, ln_x), result, (vector unsigned int)gt_r1start);

  /* Finish Stirling's Approximation */
  vec_float4 resultstirling = spu_madd(spu_sub(xabs, spu_splats(0.5f)), ln_x, halflog2pi);
  resultstirling = spu_sub(resultstirling, xabs);
  resultstirling = spu_add(spu_mul(result,inv_x), resultstirling);
  result = spu_sel(result, resultstirling, (vector unsigned int)gt_r7start);


  /* Adjust due to systematic truncation */
  result = (vec_float4)spu_add((vec_uint4)result, tcorrection);


  /*
   * Approximation for Negative X
   *
   * Use reflection relation:
   *
   * gamma(x) * gamma(-x) = -pi/(x sin(pi x))
   *
   * lgamma(x) = log(pi/(-x sin(pi x))) - lgamma(-x)
   *           
   */
  vec_float4 nresult = spu_mul(x, _sinf4(spu_mul(x, pi)));
  nresult = spu_andc(nresult, sign_maskf);
  nresult = spu_sub(logpi, spu_add(result, _logf4(nresult)));
  nresult = (vec_float4)spu_add((vec_uint4)nresult, spu_splats(1u));

  result = spu_sel(nresult, result, gt0);


  /*
   * Special Cases
   */

  /* x = non-positive integer, return infinity */
  vec_uint4 isnonposint = spu_andc(spu_cmpeq(x, xtrunc), gt0);
  result = spu_sel(result, inff, spu_or(isnonposint, spu_cmpgt(x, spu_splats(4.2e36f))));
  result = spu_sel(result, inff, spu_andc(spu_cmpeq(x, xtrunc), gt0));

  /* Zeros of function */
  result = spu_sel(result, zerof, ret_zero);

  /* x = +/- infinity or nan, return |x| */
  result   = spu_sel(result, xabs, isnaninf);


  return result;
}

#endif /* _LGAMMAF4_H_ */
#endif /* __SPU__ */
