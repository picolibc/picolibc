
#include "test.h"
#include <ieeefp.h>


/* Test fp getround and fp setround */

void
test_getround (void)
{

  newfunc("fpgetround/fpsetround");
  line(1);
  fpsetround(FP_RN);
  test_iok(fpgetround(), FP_RN);
  line(2);
  fpsetround(FP_RM);
  test_iok(fpgetround(), FP_RM);
  line(3);  
  fpsetround(FP_RP);
  test_iok(fpgetround(), FP_RP);
  line(4);  
  fpsetround(FP_RZ);
  test_iok(fpgetround(), FP_RZ);
}

/* And fpset/fpgetmask */
void
test_getmask (void)
{
  newfunc("fpsetmask/fpgetmask");
  line(1);
  fpsetmask(FP_X_INV);
  test_iok(fpgetmask(),FP_X_INV);
  line(2);
  fpsetmask(FP_X_DX);
  test_iok(fpgetmask(),FP_X_DX);
  line(3);
  fpsetmask(FP_X_OFL );
  test_iok(fpgetmask(),FP_X_OFL);
  line(4);  
  fpsetmask(FP_X_UFL);
  test_iok(fpgetmask(),FP_X_UFL);
  line(5);  
  fpsetmask(FP_X_IMP);
  test_iok(fpgetmask(),FP_X_IMP);
}

void
test_getsticky (void)
{
  newfunc("fpsetsticky/fpgetsticky");
  line(1);
  fpsetsticky(FP_X_INV);
  test_iok(fpgetsticky(),FP_X_INV);
  line(2);
  fpsetsticky(FP_X_DX);
  test_iok(fpgetsticky(),FP_X_DX);
  line(3);
  fpsetsticky(FP_X_OFL );
  test_iok(fpgetsticky(),FP_X_OFL);
  line(4);  
  fpsetsticky(FP_X_UFL);
  test_iok(fpgetsticky(),FP_X_UFL);
  line(5);  
  fpsetsticky(FP_X_IMP);
  test_iok(fpgetsticky(),FP_X_IMP);
}

void
test_getroundtoi (void)
{
  newfunc("fpsetroundtoi/fpgetroundtoi");
  line(1);
  fpsetroundtoi(FP_RDI_TOZ);
  test_iok(fpgetroundtoi(),FP_RDI_TOZ);

  line(2);
  fpsetroundtoi(FP_RDI_RD);
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
  
  fpsetround(FP_RN);
  r1 = n + m;
  test_mok(r1, add_rounded_up, 64);
  
  line(2);
  fpsetround(FP_RM);
  r2 = n + m;
  test_mok(r2, add_rounded_down, 64);
  
  fpsetround(FP_RP);
  line(3);
  r3 = n + m;
  test_mok(r3,add_rounded_up, 64);
  
  fpsetround(FP_RZ);
  line(4);
  r4 = n + m;
  test_mok(r4,add_rounded_down,64);


  fpsetround(FP_RN);
  r1 = - n - m;
  line(5);
  test_mok(r1,sub_rounded_down,64);
  
  fpsetround(FP_RM);
  r2 = - n - m;
  line(6);
  test_mok(r2,sub_rounded_down,64);


  fpsetround(FP_RP);
  r3 = - n - m;
  line(7);
  test_mok(r3,sub_rounded_up,64);

  fpsetround(FP_RZ);
  r4 = - n - m;
  line(8);
  test_mok(r4,sub_rounded_up,64);
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


