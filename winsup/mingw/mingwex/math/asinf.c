/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

/* asin = atan (x / sqrt(1 - x^2)) */

float asinf (float x)
{
  float res;

  asm (	"fld	%%st\n\t"
	"fmul	%%st(0)\n\t"			/* x^2 */
	"fld1\n\t"
	"fsubp\n\t"				/* 1 - x^2 */
	"fsqrt\n\t"				/* sqrt (1 - x^2) */
	"fpatan"
	: "=t" (res) : "0" (x) : "st(1)");
  return res;
}
