#include <math.h>
#include <errno.h>

extern long double  __QNANL;

long double
sqrtl (long double x)
{
  if (x < 0.0L )
    {
      errno = EDOM;
      return __QNANL;
    }
  else
    {
      long double res;
      asm ("fsqrt" : "=t" (res) : "0" (x));
      return res;
    }
}
