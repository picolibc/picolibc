#include <math.h>

double
round (double x)
{
  /* Add +/- 0.5 then then round towards zero.  */
  return trunc ( x + (x >= 0.0 ?  0.5 : -0.5));
}
