#include <complex.h>
double __attribute__ ((const)) creal (double complex _Z)
{
  return __real__ _Z;
}

