#include <math.h>

int
__isnan (double _x)
{
  unsigned short _sw;
  __asm__ ("fxam;"
	   "fstsw %%ax": "=a" (_sw) : "t" (_x));
  return (_sw & (FP_NAN | FP_NORMAL | FP_INFINITE | FP_ZERO | FP_SUBNORMAL))
    == FP_NAN;
}

#undef isnan
int __attribute__ ((alias ("__isnan"))) isnan (double);
