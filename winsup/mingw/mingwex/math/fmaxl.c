#include <math.h>

long double
fmaxl (long double _x, long double  _y)
{
  return (( isgreaterequal(_x, _y) || __isnanl (_y)) ?  _x : _y );
}
