#include <math.h>
#include <limits.h>
#include <errno.h>

long
lround (double x)
{
  /* Add +/- 0.5 then then round towards zero.  */
  double tmp = trunc (x + (x >= 0.0 ?  0.5 : -0.5));
  if (!isfinite (tmp) 
      || tmp > (double)LONG_MAX
      || tmp < (double)LONG_MIN)
    { 
      errno = ERANGE;
      /* Undefined behaviour, so we could return anything.  */
      /* return tmp > 0.0 ? LONG_MAX : LONG_MIN;  */
    }
  return (long)tmp;
}
