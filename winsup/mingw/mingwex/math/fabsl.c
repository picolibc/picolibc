#include <math.h>

long double
fabsl (long double x)
{
  long double res;
  asm ("fabs;" : "=t" (res) : "0" (x));
  return res;
}
