#include <math.h>

long lrintf (float x) 
{
  long retval;
  __asm__ __volatile__							      \
    ("fistpl %0"  : "=m" (retval) : "t" (x) : "st");				      \
  return retval;
}
