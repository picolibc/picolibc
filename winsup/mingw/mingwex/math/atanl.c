/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <math.h>

long double
atanl (long double x)
{
  long double res;

  asm ("fld1\n\t"
       "fpatan"
       : "=t" (res) : "0" (x));
  return res;
}
