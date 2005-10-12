/*  csqrtl.c */
/*
   Contributed by Danny Smith
   2005-01-04
*/

#include <math.h>
#include <complex.h>

long double complex  csqrtl (long double complex Z)
{
  long double complex Res;
  long double r;
  long double x = __real__ Z;
  long double y = __imag__ Z;

  if (y == 0.0L)
    {
      if (x < 0.0L)
        {
 	  __real__ Res = 0.0L;
	  __imag__ Res = sqrtl (-x);
        }
      else
        {
 	  __real__ Res = sqrtl (x);
	  __imag__ Res = 0.0L;
        }
    }

  else if (x == 0.0L)
    {
      r = sqrtl(0.5L * fabsl (y));
      __real__ Res = r;
      __imag__ Res = y > 0 ? r : -r;
    }

  else
    {
      long double t = sqrtl (2.0L * (hypotl (__real__ Z, __imag__ Z) + fabsl (x)));
      long double u = t / 2.0L;
      if ( x > 0.0L)
        {	
          __real__ Res = u;
          __imag__ Res = y / t;
        }
      else
        {
	  __real__ Res = fabsl (y / t);
	  __imag__ Res = y < 0 ? -u : u;
        }
    }

  return Res;
}
