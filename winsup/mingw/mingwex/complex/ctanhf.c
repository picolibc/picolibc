/*  ctanhf.c */

/*
   Contributed by Danny Smith
   2004-12-24
*/


#include <math.h>
#include <complex.h>
#include <errno.h>

/*
  ctanh (x + I * y) = (sinh (2 * x)  +  sin (2 * y) * I )
		     / (cosh (2 * x) + cos (2 * y)) .
*/

float complex
ctanhf (float complex Z)
{
  float complex Res;
  float two_R = 2.0f * __real__ Z;
  float two_I = 2.0f * __imag__ Z;
  float denom = coshf (two_R) + cosf (two_I);

  if (denom == 0.0f)
    {
      errno = ERANGE;
      __real__ Res = HUGE_VALF;
      __imag__ Res = HUGE_VALF;
    }
  else if (isinf (denom))
    {
      errno = ERANGE;
      __real__ Res = two_R > 0 ? 1.0f : -1.0f;
      __imag__ Res = 0.0f;
    }
  else
    {
      __real__ Res = sinhf (two_R) / denom;
      __imag__ Res = sinf (two_I) / denom;
    }
  return Res;
}
