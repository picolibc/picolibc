#include <math.h>

long long llrintf (float x) 
{
  long long retval;
  __asm__ __volatile__							      \
    ("fistpll %0"  : "=m" (retval) : "t" (x) : "st");				      \
  return retval;
}
