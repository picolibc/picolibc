#include <math.h>
#include <errno.h>

extern float  __QNANF;

float
sqrtf (float x)
{
  if (x < 0.0F )
    {
      errno = EDOM;
      return __QNANF;
    }
  else
    {
      float res;
      asm ("fsqrt" : "=t" (res) : "0" (x));
      return res;
    }
}
