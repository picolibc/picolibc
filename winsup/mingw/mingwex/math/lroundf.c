#include <math.h>
#include <limits.h>
#include <errno.h>

long
lroundf (float x)
{
  /* Add +/- 0.5, then round towards zero.  */
  float tmp = truncf (x + (x >= 0.0F ?  0.5F : -0.5F));
  if (!isfinite (tmp) 
      || tmp > (float)LONG_MAX
      || tmp < (float)LONG_MIN)
    { 
      errno = ERANGE;
      /* Undefined behaviour, so we could return anything.  */
      /* return tmp > 0.0F ? LONG_MAX : LONG_MIN;  */
    }
  return (long)tmp;
}  
