#include <complex.h>
double __attribute__ ((const)) cimag (double complex _Z)
{
  return __imag__ _Z;
}

