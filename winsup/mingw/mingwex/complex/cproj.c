/*
   cproj.c
   Contributed by Danny Smith
   2003-10-20
*/

#include <math.h>
#include <complex.h>

/* Return the value of the projection onto the Riemann sphere.*/

double complex cproj (double complex Z)
{
  complex double Res = Z;
  if (isinf (__real__ Z) || isinf (__imag__ Z))
    {
      __real__ Res = HUGE_VAL;
      __imag__ Res = copysign (0.0, __imag__ Z);
    }
  return Res;
}

