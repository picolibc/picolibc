#include <math.h>

double
fdim (double x, double y)
{
  return  (isgreater(x, y) ? (x - y) : 0.0);
}
