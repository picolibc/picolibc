#include <math.h>
float
log2f (float _x)
{
  float retval;
  __asm__ ("fyl2x;" : "=t" (retval) : "0" (_x), "u" (1.0L) : "st(1)");
  return retval;
}
