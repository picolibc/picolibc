#include <math.h>

long double
fdiml (long double x, long double y)
{
   return  (isgreater(x, y) ? (x - y) : 0.0L);
}
