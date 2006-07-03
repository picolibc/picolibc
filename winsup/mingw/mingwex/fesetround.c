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
      __asm__ volatile ("stmxcsr %0" : "=m" (_cw));
      _cw &= ~ 0x6000;
      _cw |= (mode <<  __MXCSR_ROUND_FLAG_SHIFT);
      __asm__ volatile ("ldmxcsr %0" : : "m" (_cw));
    }

  return 0;
}
