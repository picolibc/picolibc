/*
   ccosh.c
   Contributed by Danny Smith
   2003-10-20
*/

#include <math.h>
#include <complex.h>

/* ccosh (x + I * y) = cosh (x) * cos (y)
    + I * (sinh (x) * sin (y)) */ 

double complex ccosh (double complex Z)
{
  double complex Res;
  __real__ Res = cosh (__real__ Z) * cos (__imag__ Z);
  __imag__ Res = sinh (__real__ Z) * sin (__imag__ Z);
  return Res;
}
