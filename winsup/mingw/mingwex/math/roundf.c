#include <math.h>

float
roundf (float x)
{
  /* Add +/- 0.5 then then round towards zero.  */
  return truncf ( x + (x >= 0.0F ?  0.5F : -0.5F));
}
