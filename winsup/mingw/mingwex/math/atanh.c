#include <math.h>
#include <errno.h>
#include "fastmath.h"

/* atanh (x) = 0.5 * log ((1.0 + x)/(1.0 - x)) */

double atanh(double x)
{
  double z;
  if isnan (x)
    return x;
  z = fabs (x);
  if (z == 1.0)
    {
      errno  = ERANGE;
      return (x > 0 ? INFINITY : -INFINITY);
    }
  if (z > 1.0)
    {
      errno = EDOM;
      return nan("");
    }
  /* Rearrange formula to avoid precision loss for small x.

  atanh(x) = 0.5 * log ((1.0 + x)/(1.0 - x))
	   = 0.5 * log1p ((1.0 + x)/(1.0 - x) - 1.0)
           = 0.5 * log1p ((1.0 + x - 1.0 + x) /(1.0 - x)) 
           = 0.5 * log1p ((2.0 * x ) / (1.0 - x))  */
  z = 0.5 * __fast_log1p ((z + z) / (1.0 - z));
  return x >= 0 ? z : -z;
}
