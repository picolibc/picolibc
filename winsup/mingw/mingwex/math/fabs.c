#include <math.h>

double
fabs (double x)
{
  double res;

  asm ("fabs;" : "=t" (res) : "0" (x));
  return res;
}
