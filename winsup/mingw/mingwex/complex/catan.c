/* catan.c */

/*
   Contributed by Danny Smith
   2003-10-17

   FIXME: This needs some serious numerical analysis.
*/

#include <math.h>
#include <complex.h>
#include <errno.h>

/* catan (z) = -I/2 * clog ((I + z) / (I - z)) */ 

double complex 
catan (double complex Z)
{
  double complex Res;
  double complex Tmp;
  double x = __real__ Z;
  double y = __imag__ Z;

  if ( x == 0.0 && (1.0 - fabs (y)) == 0.0)
    {
      errno = ERANGE;
      __real__ Res = HUGE_VAL;
      __imag__ Res = HUGE_VAL;
    }
   else if (isinf (_hypot (x, y)))
   {
     __real__ Res = (x > 0 ? M_PI_2 : -M_PI_2);
     __imag__ Res = 0.0;
   }
  else
    {
      __real__ Tmp = - x; 
      __imag__ Tmp = 1.0 - y;

      __real__ Res = x; 
      __imag__ Res = y + 1.0;

      Tmp = clog (Res/Tmp);	
      __real__ Res  = - 0.5 * __imag__ Tmp;
      __imag__ Res =  0.5 * __real__ Tmp;
    }

   return Res; 
}
