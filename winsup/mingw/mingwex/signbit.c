#define __FP_SIGNBIT  0x0200

int __signbit (double x) {
  unsigned short sw;
  __asm__ ("fxam; fstsw %%ax;"
	   : "=a" (sw)
	   : "t" (x) );
  return sw & __FP_SIGNBIT;
}


int __signbitf (float x) {
  unsigned short sw;
  __asm__ ("fxam; fstsw %%ax;"
	   : "=a" (sw)
	   : "t" (x) );
  return sw & __FP_SIGNBIT;
}


#undef signbit
int __attribute__ ((alias ("__signbit"))) signbit (double);
int __attribute__ ((alias ("__signbitf"))) signbitf (float);
