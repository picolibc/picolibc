#include <math.h>

/* 'fxam' sets FPU flags C3,C2,C0   'fstsw' stores: 
 FP_NAN			001		0x0100
 FP_NORMAL		010		0x0400
 FP_INFINITE		011		0x0500
 FP_ZERO		100		0x4000
 FP_SUBNORMAL		110		0x4400

and sets C1 flag (signbit) if neg */

int __fpclassify (double _x){
  unsigned short sw;
  __asm__ (
	"fxam; fstsw %%ax;"
	: "=a" (sw)
	: "t" (_x)
	);
  return sw & (FP_NAN | FP_NORMAL | FP_ZERO );
}
