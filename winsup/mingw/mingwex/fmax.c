#include <math.h>


double
fmax (double _x, double _y)
{
   return ( isgreaterequal (_x, _y)|| _isnan (_y) ?  _x : _y );
}
