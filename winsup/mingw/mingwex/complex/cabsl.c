#include <math.h>
#include <complex.h>

long double cabsl (long double complex Z)
{
  return  hypotl ( __real__ Z,  __imag__ Z);
}
