#include <fenv.h>
#include <math.h>
#include <errno.h>
#define FE_ROUNDING_MASK \
  (FE_TONEAREST | FE_DOWNWARD | FE_UPWARD | FE_TOWARDZERO)

long double
modfl (long double value, long double* iptr)
{
  long double int_part;
  unsigned short saved_cw;
  unsigned short tmp_cw;
  /* truncate */ 
  asm ("fnstcw %0;" : "=m" (saved_cw)); /* save control word */
  tmp_cw = (saved_cw & ~FE_ROUNDING_MASK) | FE_TOWARDZERO;
  asm ("fldcw %0;" : : "m" (tmp_cw));
  asm ("frndint;" : "=t" (int_part) : "0" (value)); /* round */
  asm ("fldcw %0;" : : "m" (saved_cw)); /* restore saved cw */
  if (iptr)
    *iptr = int_part;
  return (isinf (value) ?  0.0L : value - int_part);
}
