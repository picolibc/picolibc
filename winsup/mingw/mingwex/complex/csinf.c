/*  csinf.c */

/*
   Contributed by Danny Smith
   2004-12-24
*/

#include <math.h>
#include <complex.h>

/* csin (x + I * y) = sin (x) * cosh (y)
    + I * (cos (x) * sinh (y)) */ 

float complex csinf (float complex Z)
{
  float complex Res;
  __real__ Res = sinf (__real__ Z) * coshf ( __imag__ Z);
  __imag__ Res = cosf (__real__ Z) * sinhf ( __imag__ Z);
  return Res;
}

