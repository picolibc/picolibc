/* csinhf.c */
/*
   Contributed by Danny Smith
   2004-12-24
*/


#include <math.h>
#include <complex.h>

/* csinh (x + I * y) = sinh (x) * cos (y)
    + I * (cosh (x) * sin (y)) */ 


float complex csinhf (float complex Z)
{
  float complex Res;
  __real__ Res = sinhf (__real__ Z) * cosf (__imag__ Z);
  __imag__ Res = coshf (__real__ Z) * sinf (__imag__ Z);
  return Res;
}
