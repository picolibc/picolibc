#include <math.h>
int __fpclassifyl (long double _x){
  unsigned short sw;
  __asm__ (
	"fxam; fstsw %%ax;"
	: "=a" (sw)
	: "t" (_x)
	);
  return sw & (FP_NAN | FP_NORMAL | FP_ZERO );
}
