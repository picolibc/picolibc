#include <math.h>

float
fabsf (float x)
{
  float res;
  asm ("fabs;" : "=t" (res) : "0" (x));
  return res;
}
