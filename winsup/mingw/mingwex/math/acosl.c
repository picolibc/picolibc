/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <math.h>

long double
acosl (long double x)
{
  long double res;

  /* acosl = atanl (sqrtl(1 - x^2) / x) */
  asm (	"fld	%%st\n\t"
	"fmul	%%st(0)\n\t"		/* x^2 */
	"fld1\n\t"
	"fsubp\n\t"			/* 1 - x^2 */
	"fsqrt\n\t"			/* sqrtl (1 - x^2) */
	"fxch	%%st(1)\n\t"
	"fpatan"
	: "=t" (res) : "0" (x) : "st(1)");
  return res;
}
