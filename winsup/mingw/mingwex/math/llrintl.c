#include <math.h>

long long llrintl (long double x) 
{
  long long retval;
  __asm__ __volatile__							      \
    ("fistpll %0"  : "=m" (retval) : "t" (x) : "st");				      \
  return retval;
}

