#include <math.h>

double
fmax (double _x, double _y)
{
   return ( isgreaterequal (_x, _y)|| __isnan (_y) ?  _x : _y );
}
