#include <fenv.h>
#include <math.h>

long
lround (double x) {
  long retval;
  unsigned short saved_cw, _cw;
  __asm__ (
	"fnstcw %0;" : "=m" (saved_cw)
	); /* save  control word  */
  _cw = ~(FE_TONEAREST | FE_DOWNWARD | FE_UPWARD | FE_TOWARDZERO)
     	  | (x > 0.0 ? FE_UPWARD : FE_DOWNWARD); /* round away from zero */
  __asm__ (
	"fldcw %0;" : : "m" (_cw)
	);  /* load the rounding control */
  __asm__ __volatile__ (
	"fistpl %0"  : "=m" (retval) : "t" (x) : "st"
	);
  __asm__ (
	"fldcw %0;" : : "m" (saved_cw)
	); /* restore control word */
  return retval;
}

