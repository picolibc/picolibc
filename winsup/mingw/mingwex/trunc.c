#include <fenv.h>
#include <math.h>

double
trunc (double _x){
  double retval;
  unsigned short saved_cw;
  __asm__ ("fnstcw %0;": "=m" (saved_cw)); /* save FPU control word */
  __asm__ ("fldcw %0;"
	    :
	    : "m" ((saved_cw & ~(FE_TONEAREST | FE_DOWNWARD | FE_UPWARD
			 | FE_TOWARDZERO)) | FE_TOWARDZERO)
	);
  __asm__ ("frndint;" : "=t" (retval)  : "0" (_x)); /* round towards zero */
  __asm__ ("fldcw %0;" : : "m" (saved_cw) ); /* restore saved control word */
  return retval;
}
