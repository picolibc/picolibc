/* catanh.c */
/*
   Contributed by Danny Smith
   2003-10-20
*/

#include <math.h>
#include <complex.h>

/*  catanh (z) = -I * catan (I  * z)  */

double complex catanh (double complex Z)
{
  double complex Tmp;
  double complex Res;

  __real__ Tmp = - __imag__ Z;
  __imag__ Tmp =   __real__ Z;
  Tmp = catan (Tmp);
  __real__ Res =  __imag__ Tmp;
  __imag__ Res = - __real__ Tmp;
  return Res;
}
