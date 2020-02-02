/*
 * Copyright (c) 1994 Cygnus Support.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
 * distribution and use acknowledge that the software was developed
 * at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "test.h"
#include <ieeefp.h>


/* Test fp getround and fp setround */

void
test_getround (void)
{

  newfunc("fpgetround/fpsetround");
  line(1);
  if (fpsetround(FP_RN) != -1)
    test_iok(fpgetround(), FP_RN);
  line(2);
  if (fpsetround(FP_RM) != -1)
    test_iok(fpgetround(), FP_RM);
  line(3);  
  if (fpsetround(FP_RP) != -1)
    test_iok(fpgetround(), FP_RP);
  line(4);  
  if (fpsetround(FP_RZ) != -1)
    test_iok(fpgetround(), FP_RZ);
}

/* And fpset/fpgetmask */
void
test_getmask (void)
{
  newfunc("fpsetmask/fpgetmask");
  line(1);
  if (fpsetmask(FP_X_INV) != -1)
    test_iok(fpgetmask(),FP_X_INV);
  line(2);
  if (fpsetmask(FP_X_DX) != -1)
    test_iok(fpgetmask(),FP_X_DX);
  line(3);
  if (fpsetmask(FP_X_OFL ) != -1)
    test_iok(fpgetmask(),FP_X_OFL);
  line(4);  
  if (fpsetmask(FP_X_UFL) != -1)
    test_iok(fpgetmask(),FP_X_UFL);
  line(5);  
  if (fpsetmask(FP_X_IMP) != -1)
    test_iok(fpgetmask(),FP_X_IMP);
}

void
test_getsticky (void)
{
  newfunc("fpsetsticky/fpgetsticky");
  line(1);
  if (fpsetsticky(FP_X_INV) != -1)
    test_iok(fpgetsticky(),FP_X_INV);
  line(2);
  if (fpsetsticky(FP_X_DX) != -1)
    test_iok(fpgetsticky(),FP_X_DX);
  line(3);
  if (fpsetsticky(FP_X_OFL ) != -1)
    test_iok(fpgetsticky(),FP_X_OFL);
  line(4);  
  if (fpsetsticky(FP_X_UFL) != -1)
    test_iok(fpgetsticky(),FP_X_UFL);
  line(5);  
  if (fpsetsticky(FP_X_IMP) != -1)
    test_iok(fpgetsticky(),FP_X_IMP);
}

void
test_getroundtoi (void)
{
  newfunc("fpsetroundtoi/fpgetroundtoi");
  line(1);
  if (fpsetroundtoi(FP_RDI_TOZ) != -1)
    test_iok(fpgetroundtoi(),FP_RDI_TOZ);

  line(2);
  if (fpsetroundtoi(FP_RDI_RD) != -1)
    test_iok(fpgetroundtoi(),FP_RDI_RD);

}

double
 dnumber (int msw,
	int lsw)
{
  
  __ieee_double_shape_type v;
  v.parts.lsw = lsw;
  v.parts.msw = msw;
  return v.value;
}

  /* Lets see if changing the rounding alters the arithmetic.
     Test by creating numbers which will have to be rounded when
     added, and seeing what happens to them */
 /* Keep them out here to stop  the compiler from folding the results */
double n;
double m;
double add_rounded_up;
double add_rounded_down;
double sub_rounded_down ;
double sub_rounded_up ;
  double r1,r2,r3,r4;
void
test_round (void)
{
  n =                dnumber(0x40000000, 0x00000008); /* near 2 */
  m =                dnumber(0x40400000, 0x00000003); /* near 3.4 */
  
  add_rounded_up   = dnumber(0x40410000, 0x00000004); /* For RN, RP */
  add_rounded_down = dnumber(0x40410000, 0x00000003); /* For RM, RZ */
  sub_rounded_down = dnumber(0xc0410000, 0x00000004); /* for RN, RM */
  sub_rounded_up   = dnumber(0xc0410000, 0x00000003); /* for RP, RZ */

  newfunc("fpsetround");
  
  line(1);
  
  if (fpsetround(FP_RN) != -1) {
    r1 = n + m;
    test_mok(r1, add_rounded_up, 64);
  }
  
  line(2);
  if (fpsetround(FP_RM) != -1) {
    r2 = n + m;
    test_mok(r2, add_rounded_down, 64);
  }
  
  line(3);
  if (fpsetround(FP_RP) != -1) {
    r3 = n + m;
    test_mok(r3,add_rounded_up, 64);
  }
  
  line(4);
  if (fpsetround(FP_RZ) != -1) {
    r4 = n + m;
    test_mok(r4,add_rounded_down,64);
  }


  line(5);
  if (fpsetround(FP_RN) != -1) {
    r1 = - n - m;
    test_mok(r1,sub_rounded_down,64);
  }
  
  line(6);
  if (fpsetround(FP_RM) != -1) {
    r2 = - n - m;
    test_mok(r2,sub_rounded_down,64);
  }


  line(7);
  if (fpsetround(FP_RP) != -1) {
    r3 = - n - m;
    test_mok(r3,sub_rounded_up,64);
  }

  line(8);
  if (fpsetround(FP_RZ) != -1) {
    r4 = - n - m;
    test_mok(r4,sub_rounded_up,64);
  }
}


void
test_ieee (void)
{
  fp_rnd old = fpgetround();
  test_getround();
  test_getmask();
  test_getsticky();
  test_getroundtoi();

  test_round();
  fpsetround(old);

  
}


