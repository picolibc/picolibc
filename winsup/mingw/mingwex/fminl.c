#include <math.h>

long double
fminl (long double _x, long double _y)
{
  return ((islessequal(_x, _y) || __isnanl (_y)) ? _x : _y );
}
