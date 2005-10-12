#include <math.h>
#include <complex.h>

float complex  csqrtf (float complex Z)
{
  float complex Res;
  float r;
  float x = __real__ Z;
  float y = __imag__ Z;

  if (y == 0.0f)
    {
      if (x < 0.0f)
        {
 	  __real__ Res = 0.0f;
	  __imag__ Res = sqrtf (-x);
        }
      else
        {
 	  __real__ Res = sqrtf (x);
	  __imag__ Res = 0.0f;
        }
    }

  else if (x == 0.0f)
    {
      r = sqrtf(0.5f * fabsf (y));
      __real__ Res = r;
      __imag__ Res = y > 0 ? r : -r;
    }

  else
    {
      float t = sqrtf (2 * (_hypot (__real__ Z, __imag__ Z) + fabsf (x)));
      float u = t / 2.0f;
      if ( x > 0.0f)
        {	
          __real__ Res = u;
          __imag__ Res = y / t;
        }
      else
        {
	  __real__ Res = fabsf (y / t);
	  __imag__ Res = y < 0 ? -u : u;
        }
    }

  return Res;
}
