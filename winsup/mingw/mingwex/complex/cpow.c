/*  cpow.c */
/*
   Contributed by Danny Smith
   2003-10-20
*/

/* cpow(X, Y) = cexp(X * clog(Y)) */

#include <math.h>
#include <complex.h>

/* Use dll version of pow */
extern double  (*_imp__pow) (double, double);
#define pow (*_imp__pow)

double complex cpow (double complex X, double complex Y)
{
  double complex Res;
  double i;
  double r = hypot (__real__ X, __imag__ X);
  if (r == 0.0)
    {
       __real__ Res = __imag__ Res = 0.0;
    }
  else
    {
      double rho;
      double theta;
      i = carg (X);
      theta = i * __real__ Y;
 
      if (__imag__ Y == 0.0)
	/* This gives slightly more accurate results in these cases. */
   	rho = pow (r, __real__ Y);
      else
	{
          r = log (r);
	  /* rearrangement of cexp(X * clog(Y)) */
	  theta += r * __imag__ Y;
	  rho = exp (r * __real__ Y - i * __imag__ Y);
	}

      __real__ Res = rho * cos (theta);
      __imag__ Res = rho * sin (theta);
    }
  return  Res;
}

