/* ctan.c */

/*
   Contributed by Danny Smith
   2003-10-20
*/

#include <math.h>
#include <complex.h>
#include <errno.h>


/* ctan (x + I * y) = (sin (2 * x)  +  I * sinh(2 * y))
		      / (cos (2 * x)  +  cosh (2 * y)) */

double complex ctan (double complex Z)
{
  double complex Res;
  double two_I = 2.0 * __imag__ Z;
  double two_R = 2.0 * __real__ Z;
  double denom = cos (two_R) + cosh (two_I);
  if (denom == 0.0)
    {
      errno = ERANGE;
      __real__ Res = HUGE_VAL;
      __imag__ Res = HUGE_VAL;
    }
  else if (isinf (denom))
    {
      errno = ERANGE;
      __real__ Res = 0.0;
      __imag__ Res = two_I > 0 ? 1.0 : -1.0;
    }
  else
    {
      __real__ Res = sin (two_R) / denom;
      __imag__ Res = sinh (two_I) / denom;
    }
  return Res;
}

