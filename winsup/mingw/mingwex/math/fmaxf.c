#include <math.h>

float
fmaxf (float _x, float _y)
{
  return (( isgreaterequal(_x, _y) || __isnanf (_y)) ?  _x : _y );
}
