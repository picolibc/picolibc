/*  cpowl.c */
/*
   Contributed by Danny Smith
   2005-01-04
*/

/* cpow(X, Y) = cexp(X * clog(Y)) */

#include <math.h>
#include <complex.h>

long double complex cpowl (long double complex X, long double complex Y)
{
  long double complex Res;
  long double i;
  long double r = hypotl (__real__ X, __imag__ X);
  if (r == 0.0L)
    {
       __real__ Res = __imag__ Res = 0.0L;
    }
  else
    {
      long double rho;
      long double theta;
      i = cargl (X);
      theta = i * __real__ Y;
 
      if (__imag__ Y == 0.0L)
	/* This gives slightly more accurate results in these cases. */
   	rho = powl (r, __real__ Y);
      else
	{
          r = logl (r);
	  /* rearrangement of cexp(X * clog(Y)) */
	  theta += r * __imag__ Y;
	  rho = expl (r * __real__ Y - i * __imag__ Y);
	}

      __real__ Res = rho * cosl (theta);
      __imag__ Res = rho * sinl (theta);
    }
  return  Res;
}
