/* csinh.c */
/*
   Contributed by Danny Smith
   2003-10-20
*/


#include <math.h>
#include <complex.h>

/* csinh (x + I * y) = sinh (x) * cos (y)
    + I * (cosh (x) * sin (y)) */ 


double complex csinh (double complex Z)
{
  double complex Res;
  __real__ Res = sinh (__real__ Z) * cos (__imag__ Z);
  __imag__ Res = cosh (__real__ Z) * sin (__imag__ Z);
  return Res;
}
