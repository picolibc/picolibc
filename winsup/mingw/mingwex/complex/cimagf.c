#include <complex.h>
float __attribute__ ((const)) cimagf (float complex _Z)
{
  return __imag__ _Z;
}

