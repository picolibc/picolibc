/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 */

#include <math.h>

float
atanf (float x)
{
  float res;

  asm ("fld1\n\t"
       "fpatan" : "=t" (res) : "0" (x));
  return res;
}
