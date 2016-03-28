#undef pow10l
#include <math.h>

long double
pow10l (long double x)
{
  return powl (10.0L, x);
}
