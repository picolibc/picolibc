/*
   cexp.c
   Contributed by Danny Smith
   2003-10-20
*/

#include <math.h>
#include <complex.h>

/* cexp (x + I * y) = exp (x) * cos (y) + I * exp (x) * sin (y) */

double complex cexp (double complex Z)
{
  double complex  Res;
  long double rho = exp (__real__ Z);
  __real__ Res = rho * cos(__imag__ Z);
  __imag__ Res = rho * sin(__imag__ Z);
  return Res;
}
