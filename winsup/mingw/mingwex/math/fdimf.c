#include <math.h>

float
fdimf (float x, float y)
{
  return  (isgreater(x, y) ? (x - y) : 0.0F);
}
