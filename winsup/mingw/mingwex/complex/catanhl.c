/* catanhl.c */
/*
   Contributed by Danny Smith
   2005-01-04
*/

#include <math.h>
#include <complex.h>

/*  catanh (z) = -I * catan (I  * z)  */

long double complex catanhl (long double complex Z)
{
  long double complex Tmp;
  long double complex Res;

  __real__ Tmp = - __imag__ Z;
  __imag__ Tmp =   __real__ Z;
  Tmp = catanl (Tmp);
  __real__ Res =  __imag__ Tmp;
  __imag__ Res = - __real__ Tmp;
  return Res;
}
