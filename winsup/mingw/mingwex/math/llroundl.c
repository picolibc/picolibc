#include <math.h>
#include <limits.h>
#include <errno.h>

long long
llroundl (long double x)
{
  /* Add +/- 0.5, then round towards zero.  */
  long double tmp = truncl (x + (x >= 0.0L ?  0.5L : -0.5L));
  if (!isfinite (tmp) 
      || tmp > (long double)LONG_LONG_MAX
      || tmp < (long double)LONG_LONG_MIN)
    { 
      errno = ERANGE;
      /* Undefined behaviour, so we could return anything.  */
      /* return tmp > 0.0L ? LONG_LONG_MAX : LONG_LONG_MIN; */
    }
  return (long long)tmp;
}
