#include <math.h>

long lrint (double x) 
{
  long retval;  
  __asm__ __volatile__							      \
    ("fistpl %0"  : "=m" (retval) : "t" (x) : "st");				      \
  return retval;
}
