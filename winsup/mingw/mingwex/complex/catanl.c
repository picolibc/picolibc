/* catanl.c */

/*
   Contributed by Danny Smith
   2005-01-04

   FIXME: This needs some serious numerical analysis.
*/

#include <math.h>
#include <complex.h>
#include <errno.h>

/* catan (z) = -I/2 * clog ((I + z) / (I - z)) */ 

#ifndef _M_PI_2L
#define _M_PI_2L 1.5707963267948966192313L
#endif

long double complex 
catanl (long double complex Z)
{
  long double complex Res;
  long double complex Tmp;
  long double x = __real__ Z;
  long double y = __imag__ Z;

  if ( x == 0.0L && (1.0L - fabsl (y)) == 0.0L)
    {
      errno = ERANGE;
      __real__ Res = HUGE_VALL;
      __imag__ Res = HUGE_VALL;
    }
   else if (isinf (hypotl (x, y)))
   {
     __real__ Res = (x > 0 ? _M_PI_2L : -_M_PI_2L);
     __imag__ Res = 0.0L;
   }
  else
    {
      __real__ Tmp = - x; 
      __imag__ Tmp = 1.0L - y;

      __real__ Res = x; 
      __imag__ Res = y + 1.0L;

      Tmp = clogl (Res/Tmp);	
      __real__ Res  = - 0.5L * __imag__ Tmp;
      __imag__ Res =  0.5L * __real__ Tmp;
    }

   return Res; 
}
