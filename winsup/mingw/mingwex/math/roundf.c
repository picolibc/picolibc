#include <fenv.h>

float
roundf (float x) {
  double retval;
  unsigned short saved_cw, _cw;
  __asm__ (
	"fnstcw %0;"
	: "=m" (saved_cw)
	); /* save  control word  */
  _cw = ~(FE_TONEAREST | FE_DOWNWARD | FE_UPWARD | FE_TOWARDZERO)
         | (x > 0.0 ? FE_UPWARD : FE_DOWNWARD); /* round away from zero */
  __asm__ (
	"fldcw %0;"
	:
	: "m" (_cw)
	);  /* load the rounding control */
  __asm__ (
	"frndint;"
	: "=t" (retval)
	: "0" (x)
	); /* do the rounding */
  __asm__ (
	"fldcw %0;"
	:
	: "m" (saved_cw)
	); /* restore control word */
  return retval;
}
