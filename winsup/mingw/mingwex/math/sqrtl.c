#include <math.h>
long double
sqrtl (long double x)
{
  long double res;
  asm ("fsqrt" : "=t" (res) : "0" (x));
  return res;
}
