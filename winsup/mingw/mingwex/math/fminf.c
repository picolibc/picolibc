#include <math.h>

float
fminf (float _x, float _y)
{
  return ((islessequal(_x, _y) || __isnanf (_y)) ? _x : _y );
}
