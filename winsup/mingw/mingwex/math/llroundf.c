#include <math.h>
#include <limits.h>
#include <errno.h>

long long
llroundf (float x)
{
  /* Add +/- 0.5, then round towards zero.  */
  float tmp = truncf (x + (x >= 0.0F ?  0.5F : -0.5F));
  if (!isfinite (tmp) 
      || tmp > (float)LONG_LONG_MAX
      || tmp < (float)LONG_LONG_MIN)
    { 
      errno = ERANGE;
      /* Undefined behaviour, so we could return anything.  */
      /* return tmp > 0.0F ? LONG_LONG_MAX : LONG_LONG_MIN; */
    }
  return (long long)tmp;
}  
