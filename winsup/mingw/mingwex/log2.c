#include <math.h>
double
log2 (double _x)
{
  double retval;
  __asm__ ("fyl2x;" : "=t" (retval) : "0" (_x), "u" (1.0L) : "st(1)");
  return retval;
}
