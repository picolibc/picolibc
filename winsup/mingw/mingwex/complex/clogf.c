/*
   clogf.c
   Contributed by Danny Smith
   2004-12-24
*/

/* clog (x + I * y) = log (hypot (x, y)) + I * atan2 (y, x) */

#include <math.h>
#include <complex.h>

float complex clogf (float complex Z)
{
  float complex Res;
  __real__ Res = logf (_hypot (__real__ Z,  __imag__ Z));
  __imag__ Res = cargf (Z);
  return Res;
}

