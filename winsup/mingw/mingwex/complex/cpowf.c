/*  cpowf.c */
/*
   Contributed by Danny Smith
   2004-12-24
*/

/* cpow(X, Y) = cexp(X * clog(Y)) */

#include <math.h>
#include <complex.h>

float complex cpowf (float complex X, float complex Y)
{
  float complex Res;
  float i;
  float r = _hypot (__real__ X, __imag__ X);
  if (r == 0.0f)
    {
       __real__ Res = __imag__ Res = 0.0;
    }
  else
    {
      float rho;
      float theta;
      i = cargf (X);
      theta = i * __real__ Y;
 
      if (__imag__ Y == 0.0f)
	/* This gives slightly more accurate results in these cases. */
   	rho = powf (r, __real__ Y);
      else
	{
          r = logf (r);
	  /* rearrangement of cexp(X * clog(Y)) */
	  theta += r * __imag__ Y;
	  rho = expf (r * __real__ Y - i * __imag__ Y);
	}

      __real__ Res = rho * cosf (theta);
      __imag__ Res = rho * sinf (theta);
    }
  return  Res;
}
