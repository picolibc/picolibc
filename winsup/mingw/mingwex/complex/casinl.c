/*
   casinl.c
   Contributed by Danny Smith
   2005-01-04
*/

#include <math.h>
#include <complex.h>

/* casin (Z ) = -I * clog(I * Z + csqrt (1.0 - Z * Z))) */

long double complex casinl (long double complex Z)
{
  long double complex Res;
  long double x, y;

  x = __real__ Z;
  y = __imag__ Z;

  if (y == 0.0L)
    {
      __real__ Res = asinl (x);
      __imag__ Res = 0.0L;
    }
  else  /* -I * clog(I * Z + csqrt(1.0 - Z * Z))) */
    {
      long double complex ZZ;
                                 
      /* Z * Z = ((x - y) * (x + y)) + (2.0 * x * y) * I */
      /* calculate 1 - Z * Z */
      __real__ ZZ = 1.0L - (x - y) * (x + y);
      __imag__ ZZ = -2.0L * x * y;
      ZZ = csqrtl (ZZ);


      /* add  I * Z  to ZZ */

      __real__ ZZ -= y;
      __imag__ ZZ += x;

      ZZ = clogl (ZZ);

      /* mult by -I */
      __real__ Res = __imag__ ZZ;
      __imag__ Res = - __real__ ZZ;  
    }
  return (Res);
}
