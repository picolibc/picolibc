/* ctanl.c */

/*
   Contributed by Danny Smith
   2005-01-04
*/

#include <math.h>
#include <complex.h>
#include <errno.h>


/* ctan (x + I * y) = (sin (2 * x)  +  I * sinh(2 * y))
		      / (cos (2 * x)  +  cosh (2 * y)) */

long double complex ctanl (long double complex Z)
{
  long double complex Res;
  long double two_I = 2.0L * __imag__ Z;
  long double two_R = 2.0L * __real__ Z;
  long double denom = cosl (two_R) + coshl (two_I);
  if (denom == 0.0L)
    {
      errno = ERANGE;
      __real__ Res = HUGE_VALL;
      __imag__ Res = HUGE_VALL;
    }
  else if (isinf (denom))
    {
      errno = ERANGE;
      __real__ Res = 0.0;
      __imag__ Res = two_I > 0 ? 1.0L : -1.0L;
    }
  else
    {
      __real__ Res = sinl (two_R) / denom;
      __imag__ Res = sinhl (two_I) / denom;
    }
  return Res;
}

