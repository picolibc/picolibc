#include <math.h>
#include <complex.h>

float cabsf (float complex Z)
{
  return (float) _hypot ( __real__ Z,  __imag__ Z);
}
