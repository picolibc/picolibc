/*
   csqrt.c
   Contributed by Danny Smith
   2003-10-20
*/

#include <math.h>
#include <complex.h>

double complex  csqrt (double complex Z)
{
  double complex Res;
  double t;
  double x = __real__ Z;
  double y = __imag__ Z;

  if (y == 0.0)
    {
      if (x < 0.0)
        {
 	  __real__ Res = 0.0;
	  __imag__ Res = sqrt (-x);
        }
      else
        {
 	  __real__ Res = sqrt (x);
	  __imag__ Res = 0.0;
        }
    }

  else if (x == 0.0)
    {
      t = sqrt(0.5 * fabs (y));
      __real__ Res = t;
      __imag__ Res = y > 0 ? t : -t;
    }

  else
    {
      t = sqrt (2.0  * (_hypot (x, y) + fabs (x)));
      double u = t / 2.0;
      if ( x > 0.0)
        {	
          __real__ Res = u;
          __imag__ Res = y / t;
        }
      else
        {
	  __real__ Res = fabs ( y / t);
	  __imag__ Res = y < 0.0 ? -u : u;
        }
    }

  return Res;
}

