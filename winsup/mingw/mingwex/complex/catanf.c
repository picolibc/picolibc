/* catanf.c */

/*
   Contributed by Danny Smith
   2004-12-24

   FIXME: This needs some serious numerical analysis.
*/

#include <math.h>
#include <complex.h>
#include <errno.h>

/* catan (z) = -I/2 * clog ((I + z) / (I - z)) */ 

float complex 
catanf (float complex Z)
{
  float complex Res;
  float complex Tmp;
  float x = __real__ Z;
  float y = __imag__ Z;

  if ( x == 0.0f && (1.0f - fabsf (y)) == 0.0f)
    {
      errno = ERANGE;
      __real__ Res = HUGE_VALF;
      __imag__ Res = HUGE_VALF;
    }
   else if (isinf (hypotf (x, y)))
   {
     __real__ Res = (x > 0 ? M_PI_2 : -M_PI_2);
     __imag__ Res = 0.0f;
   }
  else
    {
      __real__ Tmp = - x; 
      __imag__ Tmp = 1.0f - y;

      __real__ Res = x; 
      __imag__ Res = y + 1.0f;

      Tmp = clogf (Res/Tmp);	
      __real__ Res  = - 0.5f * __imag__ Tmp;
      __imag__ Res =  0.5f * __real__ Tmp;
    }

   return Res; 
}
