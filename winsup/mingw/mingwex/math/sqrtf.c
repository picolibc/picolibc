#include <math.h>

float
sqrtf (float x)
{
  float res;
  asm ("fsqrt" : "=t" (res) : "0" (x));
  return res;
}
