#include <math.h>
long double 
log2l (long double  _x)
{
  long double  retval;
  __asm__ ("fyl2x;" : "=t" (retval) : "0" (_x), "u" (1.0L) : "st(1)");
  return retval;
}
