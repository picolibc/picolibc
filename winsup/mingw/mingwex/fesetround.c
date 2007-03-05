#include <fenv.h>
#include "cpu_features.h"

 /* 7.6.3.2
    The fesetround function establishes the rounding direction
    represented by its argument round. If the argument is not equal
    to the value of a rounding direction macro, the rounding direction
    is not changed.  */

int fesetround (int mode)
{
  unsigned short _cw;
  if ((mode & ~(FE_TONEAREST | FE_DOWNWARD | FE_UPWARD | FE_TOWARDZERO))
      != 0)
    return -1;
  __asm__ volatile ("fnstcw %0;": "=m" (_cw));
  _cw &= ~(FE_TONEAREST | FE_DOWNWARD | FE_UPWARD | FE_TOWARDZERO);
  _cw |= mode;
  __asm__ volatile ("fldcw %0;" : : "m" (_cw));

 if (__HAS_SSE)
    {
      unsigned int _mxcsr;
      __asm__ volatile ("stmxcsr %0" : "=m" (_mxcsr));
      _mxcsr &= ~ 0x6000;
      _mxcsr |= (mode <<  __MXCSR_ROUND_FLAG_SHIFT);
      __asm__ volatile ("ldmxcsr %0" : : "m" (_mxcsr));
    }

  return 0;
}
