#include <math.h>
#include <limits.h>
#include <errno.h>

long long
llround (double x)
{
  /* Add +/- 0.5, then round towards zero.  */
  double tmp = trunc (x + (x >= 0.0 ?  0.5 : -0.5));
  if (!isfinite (tmp) 
      || tmp > (double)LONG_LONG_MAX
      || tmp < (double)LONG_LONG_MIN)
    { 
      errno = ERANGE;
      /* Undefined behaviour, so we could return anything.  */
      /* return tmp > 0.0 ? LONG_LONG_MAX : LONG_LONG_MIN;  */
    }
  return (long long)tmp;
}
