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
#ifndef _ERFCF4_H_
#define _ERFCF4_H_	1

#include <spu_intrinsics.h>
#include "erff4.h"
#include "erf_utils.h"
#include "recipf4.h"
#include "expf4.h"
#include "divf4.h"

/*
 * FUNCTION
 *  vector float _erfcf4(vector float x)
 *
 * DESCRIPTION
 *  The erfcf4 function computes the complement error function of each element of x.
 *
 *  C99 Special Cases:
 *  - erfc(+0) returns +1
 *  - erfc(-0) returns +1
 *  - erfc(+infinite) returns +0
 *  - erfc(-infinite) returns +2
 *
 */

static __inline vector float _erfcf4(vector float x)
{
  vec_float4 sign_maskf = spu_splats(-0.0f);
  vec_float4 zerof      = spu_splats(0.0f);
  vec_float4 onehalff   = spu_splats(0.5f);
  vec_float4 onef       = spu_splats(1.0f);
  vec_float4 twof       = spu_splats(2.0f);
  vec_float4 clamp      = spu_splats(10.0542f);   // Erfc = 0 above this (in single precision)
  vec_float4 xabs       = spu_andc(x, sign_maskf);
  vec_float4 result;

  /*
   * First thing we do is setup the description of each partition.
   * This consists of:
   * - Start x of partition
   * - Offset (used for evaluating power series expanded around a point)
   * - Truncation adjustment.
   */


  /***************************************************************
   * REGION 0: Approximation Near 0 from Above
   *
   */
#define SDM_ERFCF4_0_START     0.0f
#define SDM_ERFCF4_0_OFF       0.0f
#define SDM_ERFCF4_0_TRUNC     1u

#define SDM_ERFCF4_0_00      0.9999999999999949135f
#define SDM_ERFCF4_0_01      -1.1283791670931702608f
#define SDM_ERFCF4_0_02      -1.8051894620430502228e-10f
#define SDM_ERFCF4_0_03      0.37612639455729408814f
#define SDM_ERFCF4_0_04      -8.8929793006257568262e-8f
#define SDM_ERFCF4_0_05      -0.11283705324835578294f
#define SDM_ERFCF4_0_06      -5.4670494993502827210e-6f
#define SDM_ERFCF4_0_07      0.026889802515535093351f
#define SDM_ERFCF4_0_08      -0.000071498114084857387620f
#define SDM_ERFCF4_0_09      -0.0050714210985129775210f
#define SDM_ERFCF4_0_10      -0.00022683372291701701701f
#define SDM_ERFCF4_0_11      0.0010796064437231401311f
#define SDM_ERFCF4_0_12      -0.00012982218714593684809f
#define SDM_ERFCF4_0_13      -0.00010102962499433144847f
#define SDM_ERFCF4_0_14       0.000025784829228223517886f


  /***************************************************************
   * REGION 1: Near 1
   */
#define SDM_ERFCF4_1_START     0.88f
#define SDM_ERFCF4_1_OFF       1.125f
#define SDM_ERFCF4_1_TRUNC     1u

#define SDM_ERFCF4_1_00     0.111611768298292224f
#define SDM_ERFCF4_1_01    -0.318273958500769283f
#define SDM_ERFCF4_1_02     0.358058203313365464f
#define SDM_ERFCF4_1_03    -0.162452332984767661f
#define SDM_ERFCF4_1_04    -0.0279732971338566734f
#define SDM_ERFCF4_1_05     0.0613236836056658061f
#define SDM_ERFCF4_1_06    -0.0155368354497628942f
#define SDM_ERFCF4_1_07    -0.00960689422582997228f
#define SDM_ERFCF4_1_08     0.00603126088310672760f
#define SDM_ERFCF4_1_09     0.000360191989801368303f
#define SDM_ERFCF4_1_10    -0.00115326735470205975f
#define SDM_ERFCF4_1_11     0.000176955087857924673f
#define SDM_ERFCF4_1_12     0.000141558399011799664f
#define SDM_ERFCF4_1_13    -0.0000494556968345700811f
#define SDM_ERFCF4_1_14    0.0f


  /***************************************************************
   * REGION 2:
   */
#define SDM_ERFCF4_2_START     1.50f
#define SDM_ERFCF4_2_OFF       1.75f
#define SDM_ERFCF4_2_TRUNC     0u

#define SDM_ERFCF4_2_00     0.0133283287808175777f
#define SDM_ERFCF4_2_01    -0.0527749959301503715f
#define SDM_ERFCF4_2_02     0.0923562428777631589f
#define SDM_ERFCF4_2_03    -0.0901572847140068856f
#define SDM_ERFCF4_2_04     0.0481022098321682995f
#define SDM_ERFCF4_2_05    -0.00662436146831574865f
#define SDM_ERFCF4_2_06    -0.00896304509872736070f
#define SDM_ERFCF4_2_07     0.00605875147039124009f
#define SDM_ERFCF4_2_08    -0.000730051247140304322f
#define SDM_ERFCF4_2_09    -0.000894181745354844871f
#define SDM_ERFCF4_2_10     0.000442750499254694174f
#define SDM_ERFCF4_2_11     5.44549038611738718e-6f
#define SDM_ERFCF4_2_12    -0.0000686716770072681921f
#define SDM_ERFCF4_2_13     0.0000177205746526325771f
#define SDM_ERFCF4_2_14    0.0f


  /***************************************************************
   * REGION 3:
   */
#define SDM_ERFCF4_3_START     2.0f
#define SDM_ERFCF4_3_OFF       2.25f
#define SDM_ERFCF4_3_TRUNC     1u

#define SDM_ERFCF4_3_00      0.00146271658668117865f
#define SDM_ERFCF4_3_01     -0.00714231902201798319f
#define SDM_ERFCF4_3_02      0.0160702177995404628f
#define SDM_ERFCF4_3_03     -0.0217245536919713662f
#define SDM_ERFCF4_3_04      0.0190833836369542972f
#define SDM_ERFCF4_3_05     -0.0106576791656674587f
#define SDM_ERFCF4_3_06      0.00290435707106278173f
#define SDM_ERFCF4_3_07      0.000670455969951892490f
#define SDM_ERFCF4_3_08     -0.000999493712611392590f
#define SDM_ERFCF4_3_09      0.000369380417703939461f
#define SDM_ERFCF4_3_10      0.0000114665831641414663f
#define SDM_ERFCF4_3_11     -0.0000651349432823388933f
#define SDM_ERFCF4_3_12      0.0000226882426454011034f
#define SDM_ERFCF4_3_13      1.33207467538330703e-6f
#define SDM_ERFCF4_3_14    0.0f


  /***************************************************************
   * REGION 4:
   */
#define SDM_ERFCF4_4_START     2.46f
#define SDM_ERFCF4_4_OFF       2.75f
#define SDM_ERFCF4_4_TRUNC     1u

#define SDM_ERFCF4_4_00      0.000100621922119681351f
#define SDM_ERFCF4_4_01     -0.000586277247093792324f
#define SDM_ERFCF4_4_02      0.00161226242950792873f
#define SDM_ERFCF4_4_03     -0.00276038870506660526f
#define SDM_ERFCF4_4_04      0.00325811365963060576f
#define SDM_ERFCF4_4_05     -0.00275580841407368484f
#define SDM_ERFCF4_4_06      0.00165732740366604948f
#define SDM_ERFCF4_4_07     -0.000646040956672447276f
#define SDM_ERFCF4_4_08      0.0000890115712124397128f
#define SDM_ERFCF4_4_09      0.0000712231147231515843f
#define SDM_ERFCF4_4_10     -0.0000549969924243893176f
#define SDM_ERFCF4_4_11      0.0000158438047120425837f
#define SDM_ERFCF4_4_12      1.07113381370613701e-6f
#define SDM_ERFCF4_4_13     0.0f
#define SDM_ERFCF4_4_14     0.0f


  /***************************************************************
   * REGION 5:
   */
#define SDM_ERFCF4_5_START     2.95f
#define SDM_ERFCF4_5_OFF       3.25f
#define SDM_ERFCF4_5_TRUNC     1u

#define SDM_ERFCF4_5_00      4.30277946372736864e-6f
#define SDM_ERFCF4_5_01     -0.0000291890253835816989f
#define SDM_ERFCF4_5_02      0.0000948643324966405230f
#define SDM_ERFCF4_5_03     -0.000195809711948193862f
#define SDM_ERFCF4_5_04      0.000286569337750268210f
#define SDM_ERFCF4_5_05     -0.000313797225490890491f
#define SDM_ERFCF4_5_06      0.000263528504215059911f
#define SDM_ERFCF4_5_07     -0.000169991414511391200f
#define SDM_ERFCF4_5_08      0.0000816476305301353867f
#define SDM_ERFCF4_5_09     -0.0000259138470056606003f
#define SDM_ERFCF4_5_10      2.32886623721087698e-6f
#define SDM_ERFCF4_5_11      2.86429946075621661e-6f
#define SDM_ERFCF4_5_12      0.0f
#define SDM_ERFCF4_5_13      0.0f
#define SDM_ERFCF4_5_14      0.0f


  /***************************************************************
   * REGION 6:
   */
#define SDM_ERFCF4_6_START     3.45f
#define SDM_ERFCF4_6_OFF       3.625f
#define SDM_ERFCF4_6_TRUNC     1u

#define SDM_ERFCF4_6_00     2.95140192507759025e-7f
#define SDM_ERFCF4_6_01    -2.21592028463311237e-6f
#define SDM_ERFCF4_6_02     8.03271103179503198e-6f
#define SDM_ERFCF4_6_03    -0.0000186737448986269582f
#define SDM_ERFCF4_6_04     0.0000311685922848296785f
#define SDM_ERFCF4_6_05    -0.0000395923353434149457f
#define SDM_ERFCF4_6_06     0.0000395291139306718091f
#define SDM_ERFCF4_6_07    -0.0000315141214892874786f
#define SDM_ERFCF4_6_08     0.0000200891481859513911f
#define SDM_ERFCF4_6_09    -0.0000100551790824327187f
#define SDM_ERFCF4_6_10     3.71860071281680690e-6f
#define SDM_ERFCF4_6_11    -8.05502983594814356e-7f
#define SDM_ERFCF4_6_12    -7.67662978382552699e-8f
#define SDM_ERFCF4_6_13     1.56408548403936681e-7f
#define SDM_ERFCF4_6_14     0.0f
#define SDM_ERFCF4_6_15     0.0f
#define SDM_ERFCF4_6_16     0.0f
#define SDM_ERFCF4_6_17     0.0f


  /***************************************************************
   * REGION 7:
   */
#define SDM_ERFCF4_7_START     3.55f
#define SDM_ERFCF4_7_OFF       4.0f
#define SDM_ERFCF4_7_TRUNC     2u

#define SDM_ERFCF4_7_00     1.54172579002800189e-8f
#define SDM_ERFCF4_7_01    -1.2698234671866558e-7f
#define SDM_ERFCF4_7_02     5.0792938687466233e-7f
#define SDM_ERFCF4_7_03    -1.3121509160928777e-6f
#define SDM_ERFCF4_7_04     2.4549920365608679e-6f
#define SDM_ERFCF4_7_05    -3.5343419836695254e-6f
#define SDM_ERFCF4_7_06     4.0577914351431357e-6f
#define SDM_ERFCF4_7_07    -3.7959659297660776e-6f
#define SDM_ERFCF4_7_08     2.9264391936639771e-6f
#define SDM_ERFCF4_7_09    -1.8631747969134646e-6f
#define SDM_ERFCF4_7_10     9.702839808793979e-7f
#define SDM_ERFCF4_7_11    -4.0077792841735885e-7f
#define SDM_ERFCF4_7_12     1.2017256123590621e-7f
#define SDM_ERFCF4_7_13    -1.7432381111955779e-8f
#define SDM_ERFCF4_7_14     0.0f


  /***************************************************************
   * Now we load the description of each partition.
   */

  /* Start point for each partition */
  vec_float4 r1start = spu_splats(SDM_ERFCF4_1_START);
  vec_float4 r2start = spu_splats(SDM_ERFCF4_2_START);
  vec_float4 r3start = spu_splats(SDM_ERFCF4_3_START);
  vec_float4 r4start = spu_splats(SDM_ERFCF4_4_START);
  vec_float4 r5start = spu_splats(SDM_ERFCF4_5_START);
  vec_float4 r6start = spu_splats(SDM_ERFCF4_6_START);
  vec_float4 r7start = spu_splats(SDM_ERFCF4_7_START);

  /* X Offset for each partition */
  vec_float4 xoffseta = (vec_float4) {SDM_ERFCF4_0_OFF, SDM_ERFCF4_1_OFF, SDM_ERFCF4_2_OFF, SDM_ERFCF4_3_OFF};
  vec_float4 xoffsetb = (vec_float4) {SDM_ERFCF4_4_OFF, SDM_ERFCF4_5_OFF, SDM_ERFCF4_6_OFF, SDM_ERFCF4_7_OFF};

  /* Truncation Correction for each partition */
  vec_uint4 tcorra = (vec_uint4) {SDM_ERFCF4_0_TRUNC, SDM_ERFCF4_1_TRUNC, SDM_ERFCF4_2_TRUNC, SDM_ERFCF4_3_TRUNC};
  vec_uint4 tcorrb = (vec_uint4) {SDM_ERFCF4_4_TRUNC, SDM_ERFCF4_5_TRUNC, SDM_ERFCF4_6_TRUNC, SDM_ERFCF4_7_TRUNC};

  /* The coefficients for each partition */
  vec_float4 c00a = (vec_float4) {SDM_ERFCF4_0_00, SDM_ERFCF4_1_00, SDM_ERFCF4_2_00, SDM_ERFCF4_3_00};
  vec_float4 c01a = (vec_float4) {SDM_ERFCF4_0_01, SDM_ERFCF4_1_01, SDM_ERFCF4_2_01, SDM_ERFCF4_3_01};
  vec_float4 c02a = (vec_float4) {SDM_ERFCF4_0_02, SDM_ERFCF4_1_02, SDM_ERFCF4_2_02, SDM_ERFCF4_3_02};
  vec_float4 c03a = (vec_float4) {SDM_ERFCF4_0_03, SDM_ERFCF4_1_03, SDM_ERFCF4_2_03, SDM_ERFCF4_3_03};
  vec_float4 c04a = (vec_float4) {SDM_ERFCF4_0_04, SDM_ERFCF4_1_04, SDM_ERFCF4_2_04, SDM_ERFCF4_3_04};
  vec_float4 c05a = (vec_float4) {SDM_ERFCF4_0_05, SDM_ERFCF4_1_05, SDM_ERFCF4_2_05, SDM_ERFCF4_3_05};
  vec_float4 c06a = (vec_float4) {SDM_ERFCF4_0_06, SDM_ERFCF4_1_06, SDM_ERFCF4_2_06, SDM_ERFCF4_3_06};
  vec_float4 c07a = (vec_float4) {SDM_ERFCF4_0_07, SDM_ERFCF4_1_07, SDM_ERFCF4_2_07, SDM_ERFCF4_3_07};
  vec_float4 c08a = (vec_float4) {SDM_ERFCF4_0_08, SDM_ERFCF4_1_08, SDM_ERFCF4_2_08, SDM_ERFCF4_3_08};
  vec_float4 c09a = (vec_float4) {SDM_ERFCF4_0_09, SDM_ERFCF4_1_09, SDM_ERFCF4_2_09, SDM_ERFCF4_3_09};
  vec_float4 c10a = (vec_float4) {SDM_ERFCF4_0_10, SDM_ERFCF4_1_10, SDM_ERFCF4_2_10, SDM_ERFCF4_3_10};
  vec_float4 c11a = (vec_float4) {SDM_ERFCF4_0_11, SDM_ERFCF4_1_11, SDM_ERFCF4_2_11, SDM_ERFCF4_3_11};
  vec_float4 c12a = (vec_float4) {SDM_ERFCF4_0_12, SDM_ERFCF4_1_12, SDM_ERFCF4_2_12, SDM_ERFCF4_3_12};
  vec_float4 c13a = (vec_float4) {SDM_ERFCF4_0_13, SDM_ERFCF4_1_13, SDM_ERFCF4_2_13, SDM_ERFCF4_3_13};
  vec_float4 c14a = (vec_float4) {SDM_ERFCF4_0_14, SDM_ERFCF4_1_14, SDM_ERFCF4_2_14, SDM_ERFCF4_3_14};

  vec_float4 c00b = (vec_float4) {SDM_ERFCF4_4_00, SDM_ERFCF4_5_00, SDM_ERFCF4_6_00, SDM_ERFCF4_7_00};
  vec_float4 c01b = (vec_float4) {SDM_ERFCF4_4_01, SDM_ERFCF4_5_01, SDM_ERFCF4_6_01, SDM_ERFCF4_7_01};
  vec_float4 c02b = (vec_float4) {SDM_ERFCF4_4_02, SDM_ERFCF4_5_02, SDM_ERFCF4_6_02, SDM_ERFCF4_7_02};
  vec_float4 c03b = (vec_float4) {SDM_ERFCF4_4_03, SDM_ERFCF4_5_03, SDM_ERFCF4_6_03, SDM_ERFCF4_7_03};
  vec_float4 c04b = (vec_float4) {SDM_ERFCF4_4_04, SDM_ERFCF4_5_04, SDM_ERFCF4_6_04, SDM_ERFCF4_7_04};
  vec_float4 c05b = (vec_float4) {SDM_ERFCF4_4_05, SDM_ERFCF4_5_05, SDM_ERFCF4_6_05, SDM_ERFCF4_7_05};
  vec_float4 c06b = (vec_float4) {SDM_ERFCF4_4_06, SDM_ERFCF4_5_06, SDM_ERFCF4_6_06, SDM_ERFCF4_7_06};
  vec_float4 c07b = (vec_float4) {SDM_ERFCF4_4_07, SDM_ERFCF4_5_07, SDM_ERFCF4_6_07, SDM_ERFCF4_7_07};
  vec_float4 c08b = (vec_float4) {SDM_ERFCF4_4_08, SDM_ERFCF4_5_08, SDM_ERFCF4_6_08, SDM_ERFCF4_7_08};
  vec_float4 c09b = (vec_float4) {SDM_ERFCF4_4_09, SDM_ERFCF4_5_09, SDM_ERFCF4_6_09, SDM_ERFCF4_7_09};
  vec_float4 c10b = (vec_float4) {SDM_ERFCF4_4_10, SDM_ERFCF4_5_10, SDM_ERFCF4_6_10, SDM_ERFCF4_7_10};
  vec_float4 c11b = (vec_float4) {SDM_ERFCF4_4_11, SDM_ERFCF4_5_11, SDM_ERFCF4_6_11, SDM_ERFCF4_7_11};
  vec_float4 c12b = (vec_float4) {SDM_ERFCF4_4_12, SDM_ERFCF4_5_12, SDM_ERFCF4_6_12, SDM_ERFCF4_7_12};
  vec_float4 c13b = (vec_float4) {SDM_ERFCF4_4_13, SDM_ERFCF4_5_13, SDM_ERFCF4_6_13, SDM_ERFCF4_7_13};
  vec_float4 c14b = (vec_float4) {SDM_ERFCF4_4_14, SDM_ERFCF4_5_14, SDM_ERFCF4_6_14, SDM_ERFCF4_7_14};

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
  vec_uchar16 gt_r1start = (vec_uchar16)spu_cmpabsgt(x, r1start);
  vec_uchar16 gt_r2start = (vec_uchar16)spu_cmpabsgt(x, r2start);
  vec_uchar16 gt_r3start = (vec_uchar16)spu_cmpabsgt(x, r3start);
  vec_uchar16 gt_r4start = (vec_uchar16)spu_cmpabsgt(x, r4start);
  vec_uchar16 gt_r5start = (vec_uchar16)spu_cmpabsgt(x, r5start);
  vec_uchar16 gt_r6start = (vec_uchar16)spu_cmpabsgt(x, r6start);
  vec_uchar16 gt_r7start = (vec_uchar16)spu_cmpabsgt(x, r7start);

  vec_uchar16 shufflepattern;
  shufflepattern = spu_sel(shuffle0, shuffle1, gt_r1start);
  shufflepattern = spu_sel(shufflepattern, shuffle2, gt_r2start);
  shufflepattern = spu_sel(shufflepattern, shuffle3, gt_r3start);
  shufflepattern = spu_sel(shufflepattern, shuffle4, gt_r4start);
  shufflepattern = spu_sel(shufflepattern, shuffle5, gt_r5start);
  shufflepattern = spu_sel(shufflepattern, shuffle6, gt_r6start);
  shufflepattern = spu_sel(shufflepattern, shuffle7, gt_r7start);


  /* Use the shuffle pattern to select the coefficients */
  vec_float4 coeff_14 = spu_shuffle(c14a, c14b, shufflepattern);
  vec_float4 coeff_13 = spu_shuffle(c13a, c13b, shufflepattern);
  vec_float4 coeff_12 = spu_shuffle(c12a, c12b, shufflepattern);
  vec_float4 coeff_11 = spu_shuffle(c11a, c11b, shufflepattern);
  vec_float4 coeff_10 = spu_shuffle(c10a, c10b, shufflepattern);
  vec_float4 coeff_09 = spu_shuffle(c09a, c09b, shufflepattern);
  vec_float4 coeff_08 = spu_shuffle(c08a, c08b, shufflepattern);
  vec_float4 coeff_07 = spu_shuffle(c07a, c07b, shufflepattern);
  vec_float4 coeff_06 = spu_shuffle(c06a, c06b, shufflepattern);
  vec_float4 coeff_05 = spu_shuffle(c05a, c05b, shufflepattern);
  vec_float4 coeff_04 = spu_shuffle(c04a, c04b, shufflepattern);
  vec_float4 coeff_03 = spu_shuffle(c03a, c03b, shufflepattern);
  vec_float4 coeff_02 = spu_shuffle(c02a, c02b, shufflepattern);
  vec_float4 coeff_01 = spu_shuffle(c01a, c01b, shufflepattern);
  vec_float4 coeff_00 = spu_shuffle(c00a, c00b, shufflepattern);

  vec_float4 xoffset     = spu_shuffle(xoffseta, xoffsetb, shufflepattern);
  vec_uint4  tcorrection = spu_shuffle(tcorra,   tcorrb,   shufflepattern);


  /*
   * We've completed the coeff. setup. Now we actually do the
   * approximation below.
   */

  /* Adjust x value here (for approximations about a point) */
  vec_float4 xappr = spu_sub(xabs, xoffset);


  /* Now we do the multiplies.
   * Use Horner's method.
   */
  result = coeff_14;
  result = spu_madd(xappr, result, coeff_13);
  result = spu_madd(xappr, result, coeff_12);
  result = spu_madd(xappr, result, coeff_11);
  result = spu_madd(xappr, result, coeff_10);
  result = spu_madd(xappr, result, coeff_09);
  result = spu_madd(xappr, result, coeff_08);
  result = spu_madd(xappr, result, coeff_07);
  result = spu_madd(xappr, result, coeff_06);
  result = spu_madd(xappr, result, coeff_05);
  result = spu_madd(xappr, result, coeff_04);
  result = spu_madd(xappr, result, coeff_03);
  result = spu_madd(xappr, result, coeff_02);
  result = spu_madd(xappr, result, coeff_01);
  result = spu_madd(xappr, result, coeff_00);

  /* Adjust due to systematic truncation. */
  result = (vec_float4)spu_add((vec_uint4)result, tcorrection);

  /* Use the continued fraction approximation for x above approx. 4
   * and below approx. 10
   */
  vec_float4 presult, xsqu;
  xsqu = spu_mul(x, x);
  CONTFRAC_ERFCF4(xabs, xsqu, presult);

  /* Select between polynomial and continued fraction */
  result = spu_sel(presult, result, spu_cmpgt(spu_splats(4.3f), xabs));

  /* Above clamp value, set erfc = 0 */
  result = spu_sel(result, zerof, spu_cmpgt(xabs, clamp));

  /* Negative x values */
  vec_uint4 gt0 = spu_cmpgt(x, zerof);
  result = spu_sel(spu_sub(twof, result), result, gt0);

  return result;
}

#endif /* _ERFCF4_H_ */
#endif /* __SPU__ */
