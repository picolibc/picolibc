#include <complex.h>
long double   __attribute__ ((const)) cargl (long double _Complex _Z)
{
  long double res;
  __asm__ ("fpatan;"
	   : "=t" (res) : "0" (__real__ _Z), "u" (__imag__ _Z) : "st(1)");
  return res;
}
