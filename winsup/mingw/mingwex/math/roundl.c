#include <math.h>

long double
roundl (long double x)
{
  /* Add +/- 0.5 then then round towards zero.  */
  return truncl ( x + (x >= 0.0L ?  0.5L : -0.5L));
}
