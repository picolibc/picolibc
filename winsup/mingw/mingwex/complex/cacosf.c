/*
   cacosf.c
   Contributed by Danny Smith
   2004-12-24
*/

#include <math.h>
#include <complex.h>

#if 0
/* cacos (Z) = -I * clog(Z + I * csqrt(1 - Z * Z)) */

float complex cacos (float  complex Z)
{
  float complex Res;
  float x, y;

  x = __real__ Z;
  y = __imag__ Z;

  if (y == 0.0f)
    {
      __real__ Res = acosf (x);
      __imag__ Res = 0.0f;
    }

  else
    {
      float complex ZZ;
      /* Z * Z = ((x - y) * (x + y)) + (2.0 * x * y) * I */
      /* caculate 1 - Z * Z */
      __real__ ZZ = 1.0f - (x - y) * (x + y);
      __imag__ ZZ = -2.0f * x * y;

       
       Res = csqrtf(ZZ);

      /* calculate ZZ + I * sqrt (ZZ) */
    
      __real__ ZZ = x - __imag__ Res;
      __imag__ ZZ = y + __real__ Res;
       
      ZZ = clog(ZZ);

      /* mult by -I */

      __real__ Res  =  __imag__ ZZ;
      __imag__ Res = - __real__ ZZ;
    }
  return Res;
}

#else

/* cacos ( Z ) =  pi/2 - casin ( Z ) */

float complex cacosf (float complex Z)
{
  float complex Res  = casinf (Z);
  __real__ Res = M_PI_2 - __real__ Res;
  __imag__ Res = - __imag__ Res;
  return Res;
}
#endif
