/*
   ccos.c
   Contributed by Danny Smith
   2003-10-20
*/

#include <math.h>
#include <complex.h>

/* ccos (x + I * y) = cos (x) * cosh (y)
    + I * (sin (x) * sinh (y)) */ 


double complex ccos (double complex Z)
{
  double complex Res;
  __real__ Res = cos (__real__ Z) * cosh ( __imag__ Z);
  __imag__ Res = -sin (__real__ Z) * sinh ( __imag__ Z);
  return Res;
}
