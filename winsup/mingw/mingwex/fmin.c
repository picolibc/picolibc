#include <math.h>

double
fmin (double _x, double _y)
{
  return ((islessequal(_x, _y) || __isnan (_y)) ? _x : _y );
}
