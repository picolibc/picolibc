/* ctanf.c */

/*
   Contributed by Danny Smith
   2004-12-24
*/

#include <math.h>
#include <complex.h>
#include <errno.h>


/* ctan (x + I * y) = (sin (2 * x)  +  I * sinh(2 * y))
		      / (cos (2 * x)  +  cosh (2 * y)) */

float complex ctanf (float complex Z)
{
  float complex Res;
  float two_I = 2.0f * __imag__ Z;
  float two_R = 2.0f * __real__ Z;
  float denom = cosf (two_R) + coshf (two_I);
  if (denom == 0.0f)
    {
      errno = ERANGE;
      __real__ Res = HUGE_VALF;
      __imag__ Res = HUGE_VALF;
    }
  else if (isinf (denom))
    {
      errno = ERANGE;
      __real__ Res = 0.0;
      __imag__ Res = two_I > 0 ? 1.0f : -1.0f;
    }
  else
    {
      __real__ Res = sinf (two_R) / denom;
      __imag__ Res = sinhf (two_I) / denom;
    }
  return Res;
}

