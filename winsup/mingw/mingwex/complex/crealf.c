#include <complex.h>
float __attribute__ ((const)) crealf (float complex _Z)
{
  return __real__ _Z;
}

