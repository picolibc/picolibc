/*
   cacos.c
   Contributed by Danny Smith
   2003-10-20
*/

#include <math.h>
#include <complex.h>

#if 0
/* cacos (Z) = -I * clog(Z + I * csqrt(1 - Z * Z)) */

double complex cacos (double  complex Z)
{
  double complex Res;
  double x, y;

  x = __real__ Z;
  y = __imag__ Z;

  if (y == 0.0)
    {
      __real__ Res = acos (x);
      __imag__ Res = 0.0;
    }

  else
    {
      double complex ZZ;
      /* Z * Z = ((x - y) * (x + y)) + (2.0 * x * y) * I */
      /* caculate 1 - Z * Z */
      __real__ ZZ = 1.0 - (x - y) * (x + y);
      __imag__ ZZ = -2.0 * x * y;

       
       Res = csqrt(ZZ);

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

double complex cacos (double complex Z)
{
  double complex Res  = casin (Z);
  __real__ Res = M_PI_2 - __real__ Res;
  __imag__ Res = - __imag__ Res;
  return Res;
}
#endif

#if 0
#include <stdio.h>
int main()
{
  double z;
  double complex bar = 0.7 + 1.2  * I;
  double complex foo = cacos (bar);

  printf ("%.16e\t%.16e\n", __real__ foo, __imag__ foo);

  foo = cacos (bar);
  printf ("%.16e\t%.16e\n", __real__ foo, __imag__ foo);

  return 1;
}
#endif

