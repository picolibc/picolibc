/*
   ccosl.c
   Contributed by Danny Smith
   2005-01-04
*/

#include <math.h>
#include <complex.h>

/* ccos (x + I * y) = cos (x) * cosh (y)
    + I * (sin (x) * sinh (y)) */ 


long double complex ccosl (long double complex Z)
{
  long double complex Res;
  __real__ Res = cosl (__real__ Z) * coshl ( __imag__ Z);
  __imag__ Res = -sinl (__real__ Z) * sinhl ( __imag__ Z);
  return Res;
}
