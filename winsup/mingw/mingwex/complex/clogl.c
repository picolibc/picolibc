/*
   clogl.c
   Contributed by Danny Smith
   2005-01-04
*/

/* clog (x + I * y) = log (hypot (x, y)) + I * atan2 (y, x) */

#include <math.h>
#include <complex.h>

long double complex clogl (long double complex Z)
{
  long double complex Res;
  __real__ Res = logl (hypotl (__real__ Z,  __imag__ Z));
  __imag__ Res = cargl (Z);
  return Res;
}

