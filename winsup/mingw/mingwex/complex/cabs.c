#include <math.h>
#include <complex.h>

double cabs (double complex Z)
{
  return _hypot ( __real__ Z,  __imag__ Z);
}
