/*
   ccoshl.c
   Contributed by Danny Smith
   2005-01-04
*/

#include <math.h>
#include <complex.h>

/* ccosh (x + I * y) = cosh (x) * cos (y)
    + I * (sinh (x) * sin (y)) */ 

long double complex ccoshl (long double complex Z)
{
  long double complex Res;
  __real__ Res = coshl (__real__ Z) * cosl (__imag__ Z);
  __imag__ Res = sinhl (__real__ Z) * sinl (__imag__ Z);
  return Res;
}
