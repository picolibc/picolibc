#include <fenv.h>

/* 7.6.3.1
   The fegetround function returns the value of the rounding direction
   macro representing the current rounding direction.  */

int
fegetround (void)
{
  unsigned short _cw;
  __asm__ ("fnstcw %0;"	: "=m" (_cw));
  return _cw
          & (FE_TONEAREST | FE_DOWNWARD | FE_UPWARD | FE_TOWARDZERO);
}
