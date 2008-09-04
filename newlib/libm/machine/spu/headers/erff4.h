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
#ifndef _ERFF4_H_
#define _ERFF4_H_	1

#include <spu_intrinsics.h>

/*
 * FUNCTION
 *  vector float _erff4(vector float x)
 *
 * DESCRIPTION
 *  The erff4 function computes the error function of each element of x.
 *
 *  C99 Special Cases:
 *  - erf(+0) returns +0
 *  - erf(-0) returns -0
 *  - erf(+infinite) returns +1
 *  - erf(-infinite) returns -1
 *
 */

static __inline vector float _erff4(vector float x)
{
  vec_float4 sign_maskf = spu_splats(-0.0f);
  vec_float4 zerof      = spu_splats(0.0f);
  vec_float4 onef       = spu_splats(1.0f);
  vec_float4 clamp      = spu_splats(3.9199876f);
  vec_float4 xabs       = spu_andc(x, sign_maskf);
  vec_float4 xsign      = spu_and(x, sign_maskf);
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
#define SDM_ERFF4_0_START     0.0f
#define SDM_ERFF4_0_OFF       0.0f
#define SDM_ERFF4_0_TRUNC     2u

#define SDM_ERFF4_0_00   0.0f
#define SDM_ERFF4_0_01   1.12837916709551257389615890312154f
#define SDM_ERFF4_0_02   0.0f
#define SDM_ERFF4_0_03  -0.37612638903183752463205296770955f
#define SDM_ERFF4_0_04   0.0f
#define SDM_ERFF4_0_05   0.11283791670955125738961589031073f
#define SDM_ERFF4_0_06   0.0f
#define SDM_ERFF4_0_07  -0.02686617064513125175943235483588f
#define SDM_ERFF4_0_08   0.0f
#define SDM_ERFF4_0_09   0.00522397762544218784211184677371f
#define SDM_ERFF4_0_10   0.0f
//#define SDM_ERFF4_0_11  -0.00085483270234508528325466583569f



  /***************************************************************
   * REGION 1: Above 0 and Below 1
   */
#define SDM_ERFF4_1_START     0.07f
#define SDM_ERFF4_1_OFF       0.0625f
#define SDM_ERFF4_1_TRUNC     1u

#define SDM_ERFF4_1_00     0.0704319777223870780505900559232967439190042883f
#define SDM_ERFF4_1_01     1.1239800336253906104888456836298420746260842545f
#define SDM_ERFF4_1_02    -0.0702487521015869131555528552268651296641302713f
#define SDM_ERFF4_1_03    -0.3717329798708974154481338589088279778060226856f
#define SDM_ERFF4_1_04     0.0350329063214945152846051348331892508611482993f
#define SDM_ERFF4_1_05     0.1106440713032318617523250293018186620702780982f
#define SDM_ERFF4_1_06    -0.0116471931712158678624014740659716890227703402f
#define SDM_ERFF4_1_07    -0.0261358409084263503958678377968739965222786482f
#define SDM_ERFF4_1_08     0.0029041996223118476954500365511415181291113910f
#define SDM_ERFF4_1_09     0.0050416329596619035812041623972929782386498567f
#define SDM_ERFF4_1_10    -0.0005793225670734356072895029723913210064918149f
//#define SDM_ERFF4_1_11    -0.0008184112733188406359323913130525859730689332f



  /***************************************************************
   * REGION 2:
   */
#define SDM_ERFF4_2_START     0.13f
#define SDM_ERFF4_2_OFF       0.1875f
#define SDM_ERFF4_2_TRUNC     1u

#define SDM_ERFF4_2_00    0.2091176770593758483008706390019410965937912290f
#define SDM_ERFF4_2_01    1.0893988034775673230502318110338693557898033315f
#define SDM_ERFF4_2_02   -0.2042622756520438730719184645688505042105881396f
#define SDM_ERFF4_2_03   -0.3376001500360169568827541289401834722369442864f
#define SDM_ERFF4_2_04    0.0997374392832245473983976877777590352590762400f
#define SDM_ERFF4_2_05    0.0937997370645632460099464120987231140266525679f
#define SDM_ERFF4_2_06   -0.0324591340420617488485277008302392706957527828f
#define SDM_ERFF4_2_07   -0.0205943885488331791711970665266474471714543313f
#define SDM_ERFF4_2_08    0.0079208906865255014554772269570592999495375181f
#define SDM_ERFF4_2_09    0.0036744273281123333893101007014150883409965011f
#define SDM_ERFF4_2_10   -0.0015459493690754127608506357908913858038162608f
//#define SDM_ERFF4_2_11   -0.0005485671070180836650399266219057172124875094f



  /***************************************************************
   * REGION 3:
   */
#define SDM_ERFF4_3_START     0.25f
#define SDM_ERFF4_3_OFF       0.5f
#define SDM_ERFF4_3_TRUNC     2u

#define SDM_ERFF4_3_00    0.5204998778130465376827466538919645287364515699f
#define SDM_ERFF4_3_01    0.8787825789354447940937239548244578983625218956f
#define SDM_ERFF4_3_02   -0.4393912894677223970468619774122289491812609947f
#define SDM_ERFF4_3_03   -0.1464637631559074656822873258040763163937536583f
#define SDM_ERFF4_3_04    0.1830797039448843321028591572550953954921920811f
#define SDM_ERFF4_3_05    0.0073231881577953732841143662902038158196876832f
#define SDM_ERFF4_3_06   -0.0500417857449350507747815029830594081011991688f
#define SDM_ERFF4_3_07    0.0054052103069442040906558417856266259621504328f
#define SDM_ERFF4_3_08    0.0100475885141180567975497704160236877764167320f
#define SDM_ERFF4_3_09   -0.0021674118390300459951330548378744759122422210f
#define SDM_ERFF4_3_10   -0.0015694967741624277200510981457278746801387524f
//#define SDM_ERFF4_3_11    0.0004973489167651373192082360776274483020158863f



  /***************************************************************
   * REGION 4:
   */
#define SDM_ERFF4_4_START     0.77f
#define SDM_ERFF4_4_OFF       1.0f
#define SDM_ERFF4_4_TRUNC     1u

#define SDM_ERFF4_4_00     0.8427007929497148693412206350826092590442f
#define SDM_ERFF4_4_01     0.4151074974205947033402682494413373653605f
#define SDM_ERFF4_4_02    -0.4151074974205947033402682494413373653605f
#define SDM_ERFF4_4_03     0.1383691658068649011134227498137791217898f
#define SDM_ERFF4_4_04     0.0691845829034324505567113749068895608946f
#define SDM_ERFF4_4_05    -0.0691845829034324505567113749068895608946f
#define SDM_ERFF4_4_06     0.0046123055268954967037807583271259707263f
#define SDM_ERFF4_4_07     0.0151547181597994891695653487891281895293f
#define SDM_ERFF4_4_08    -0.0047770307242846215860586425530947553951f
#define SDM_ERFF4_4_09    -0.0018851883701199847638468972527538689873f
#define SDM_ERFF4_4_10     0.0012262875805634852347353603488787303121f
//#define SDM_ERFF4_4_11     0.0000855239913717274641321540324726821411f



  /***************************************************************
   * REGION 5:
   */
#define SDM_ERFF4_5_START     1.36f
#define SDM_ERFF4_5_OFF       1.875f
#define SDM_ERFF4_5_TRUNC     1u

#define SDM_ERFF4_5_00     0.99199005767011997029646305969122440092668f
#define SDM_ERFF4_5_01     0.03354582842421607459425032786195496507386f
#define SDM_ERFF4_5_02    -0.06289842829540513986421936474116555951979f
#define SDM_ERFF4_5_03     0.06744109256118439996552409663913862770819f
#define SDM_ERFF4_5_04    -0.04225988151097532834627238568547061029869f
#define SDM_ERFF4_5_05     0.01146258336487617627004706027236136941544f
#define SDM_ERFF4_5_06     0.00410518713321247739022655684589964019683f
#define SDM_ERFF4_5_07    -0.00492839390823910723763257456562751425198f
#define SDM_ERFF4_5_08     0.00143050168737012207687743571780226012058f
#define SDM_ERFF4_5_09     0.00036225644575338665306295794978774160986f
#define SDM_ERFF4_5_10    -0.00039015757824554169745459780322413823624f
//#define SDM_ERFF4_5_11     0.00007372993782406230817649249567932577159f



  /***************************************************************
   * REGION 6:
   */
#define SDM_ERFF4_6_START     2.0f
#define SDM_ERFF4_6_OFF       2.5f
#define SDM_ERFF4_6_TRUNC     1u

#define SDM_ERFF4_6_00    0.999593047982555041060435784260025087279f
#define SDM_ERFF4_6_01    0.002178284230352709720386678564097264007f
#define SDM_ERFF4_6_02   -0.005445710575881774300966696410243160031f
#define SDM_ERFF4_6_03    0.008350089549685387261482267829039512051f
#define SDM_ERFF4_6_04   -0.008622375078479475976530602649551670054f
#define SDM_ERFF4_6_05    0.006117348213573859798085922300839816434f
#define SDM_ERFF4_6_06   -0.002798490157050356237996774544152735014f
#define SDM_ERFF4_6_07    0.000542410061327906884739143174194854432f
#define SDM_ERFF4_6_08    0.000260670173895134533751630061303802055f
#define SDM_ERFF4_6_09   -0.000250285386311056635227961206817778392f
#define SDM_ERFF4_6_10    0.000078801328907504400502579703621546608f
//#define SDM_ERFF4_6_11    5.137004620216358263402877651297096663210e-6f



  /***************************************************************
   * REGION 7:
   */
#define SDM_ERFF4_7_START     2.75f
#define SDM_ERFF4_7_OFF       3.5f
#define SDM_ERFF4_7_TRUNC     1u

#define SDM_ERFF4_7_00    0.999999256901627658587254476316243904363263f
#define SDM_ERFF4_7_01    5.399426777384782511586818937495781413007869e-6f
#define SDM_ERFF4_7_02   -0.000018897993720846738790553866281235234945f
#define SDM_ERFF4_7_03    0.000042295509756180796340763415010383621069f
#define SDM_ERFF4_7_04   -0.000067717810833034147332818020841092925222f
#define SDM_ERFF4_7_05    0.000082116282239393567363716204674415008991f
#define SDM_ERFF4_7_06   -0.000077744246390483389302250766562526063763f
#define SDM_ERFF4_7_07    0.000058192750619199206596604051163855823527f
#define SDM_ERFF4_7_08   -0.000034259175422410008064403380504975403351f
#define SDM_ERFF4_7_09    0.000015330768263696827211862952666453348031f
#define SDM_ERFF4_7_10   -4.641017709492666503521243665632827470977627e-6f
//#define SDM_ERFF4_7_11    4.447037356176705948450355327103423490366212e-7f





  /***************************************************************
   * Now we load the description of each partition.
   */

  /* Start point for each partition */
  vec_float4 r1start = spu_splats(SDM_ERFF4_1_START);
  vec_float4 r2start = spu_splats(SDM_ERFF4_2_START);
  vec_float4 r3start = spu_splats(SDM_ERFF4_3_START);
  vec_float4 r4start = spu_splats(SDM_ERFF4_4_START);
  vec_float4 r5start = spu_splats(SDM_ERFF4_5_START);
  vec_float4 r6start = spu_splats(SDM_ERFF4_6_START);
  vec_float4 r7start = spu_splats(SDM_ERFF4_7_START);

  /* X Offset for each partition */
  vec_float4 xoffseta = (vec_float4) {SDM_ERFF4_0_OFF, SDM_ERFF4_1_OFF, SDM_ERFF4_2_OFF, SDM_ERFF4_3_OFF};
  vec_float4 xoffsetb = (vec_float4) {SDM_ERFF4_4_OFF, SDM_ERFF4_5_OFF, SDM_ERFF4_6_OFF, SDM_ERFF4_7_OFF};

  /* Truncation Correction for each partition */
  vec_uint4 tcorra = (vec_uint4) {SDM_ERFF4_0_TRUNC, SDM_ERFF4_1_TRUNC, SDM_ERFF4_2_TRUNC, SDM_ERFF4_3_TRUNC};
  vec_uint4 tcorrb = (vec_uint4) {SDM_ERFF4_4_TRUNC, SDM_ERFF4_5_TRUNC, SDM_ERFF4_6_TRUNC, SDM_ERFF4_7_TRUNC};

  /* The coefficients for each partition */
  vec_float4 c00a = (vec_float4) {SDM_ERFF4_0_00, SDM_ERFF4_1_00, SDM_ERFF4_2_00, SDM_ERFF4_3_00};
  vec_float4 c01a = (vec_float4) {SDM_ERFF4_0_01, SDM_ERFF4_1_01, SDM_ERFF4_2_01, SDM_ERFF4_3_01};
  vec_float4 c02a = (vec_float4) {SDM_ERFF4_0_02, SDM_ERFF4_1_02, SDM_ERFF4_2_02, SDM_ERFF4_3_02};
  vec_float4 c03a = (vec_float4) {SDM_ERFF4_0_03, SDM_ERFF4_1_03, SDM_ERFF4_2_03, SDM_ERFF4_3_03};
  vec_float4 c04a = (vec_float4) {SDM_ERFF4_0_04, SDM_ERFF4_1_04, SDM_ERFF4_2_04, SDM_ERFF4_3_04};
  vec_float4 c05a = (vec_float4) {SDM_ERFF4_0_05, SDM_ERFF4_1_05, SDM_ERFF4_2_05, SDM_ERFF4_3_05};
  vec_float4 c06a = (vec_float4) {SDM_ERFF4_0_06, SDM_ERFF4_1_06, SDM_ERFF4_2_06, SDM_ERFF4_3_06};
  vec_float4 c07a = (vec_float4) {SDM_ERFF4_0_07, SDM_ERFF4_1_07, SDM_ERFF4_2_07, SDM_ERFF4_3_07};
  vec_float4 c08a = (vec_float4) {SDM_ERFF4_0_08, SDM_ERFF4_1_08, SDM_ERFF4_2_08, SDM_ERFF4_3_08};
  vec_float4 c09a = (vec_float4) {SDM_ERFF4_0_09, SDM_ERFF4_1_09, SDM_ERFF4_2_09, SDM_ERFF4_3_09};
  vec_float4 c10a = (vec_float4) {SDM_ERFF4_0_10, SDM_ERFF4_1_10, SDM_ERFF4_2_10, SDM_ERFF4_3_10};

  vec_float4 c00b = (vec_float4) {SDM_ERFF4_4_00, SDM_ERFF4_5_00, SDM_ERFF4_6_00, SDM_ERFF4_7_00};
  vec_float4 c01b = (vec_float4) {SDM_ERFF4_4_01, SDM_ERFF4_5_01, SDM_ERFF4_6_01, SDM_ERFF4_7_01};
  vec_float4 c02b = (vec_float4) {SDM_ERFF4_4_02, SDM_ERFF4_5_02, SDM_ERFF4_6_02, SDM_ERFF4_7_02};
  vec_float4 c03b = (vec_float4) {SDM_ERFF4_4_03, SDM_ERFF4_5_03, SDM_ERFF4_6_03, SDM_ERFF4_7_03};
  vec_float4 c04b = (vec_float4) {SDM_ERFF4_4_04, SDM_ERFF4_5_04, SDM_ERFF4_6_04, SDM_ERFF4_7_04};
  vec_float4 c05b = (vec_float4) {SDM_ERFF4_4_05, SDM_ERFF4_5_05, SDM_ERFF4_6_05, SDM_ERFF4_7_05};
  vec_float4 c06b = (vec_float4) {SDM_ERFF4_4_06, SDM_ERFF4_5_06, SDM_ERFF4_6_06, SDM_ERFF4_7_06};
  vec_float4 c07b = (vec_float4) {SDM_ERFF4_4_07, SDM_ERFF4_5_07, SDM_ERFF4_6_07, SDM_ERFF4_7_07};
  vec_float4 c08b = (vec_float4) {SDM_ERFF4_4_08, SDM_ERFF4_5_08, SDM_ERFF4_6_08, SDM_ERFF4_7_08};
  vec_float4 c09b = (vec_float4) {SDM_ERFF4_4_09, SDM_ERFF4_5_09, SDM_ERFF4_6_09, SDM_ERFF4_7_09};
  vec_float4 c10b = (vec_float4) {SDM_ERFF4_4_10, SDM_ERFF4_5_10, SDM_ERFF4_6_10, SDM_ERFF4_7_10};


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
  result = coeff_10;
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


  /* Adjust due to systematic truncation. Note that the correction
   * value is always non-negative, so the result is cast as uint
   * to do the adjustment.
   */
  result = (vec_float4)spu_add((vec_uint4)result, tcorrection);


  /*
   * Special Cases
   */

  /* Erf(0) = 0 */
  result = spu_sel(result, zerof, spu_cmpeq(xabs, zerof));

  /* Erf(infinity) = 1 */
  result = spu_sel(result, onef, spu_cmpgt(xabs, clamp));


  /* Preserve sign in result, since erf(-x) = -erf(x) */
  result = spu_or(result, xsign);

  return result;
}

#endif /* _ERFF4_H_ */
#endif /* __SPU__ */
