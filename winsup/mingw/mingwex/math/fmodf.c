/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for float type by Danny Smith
 *  <dannysmith@users.sourceforge.net>.
 */

#include <math.h>

float
fmodf (float x, float y)
{
  float res;

  asm ("1:\tfprem\n\t"
       "fstsw   %%ax\n\t"
       "sahf\n\t"
       "jp      1b\n\t"
       "fstp    %%st(1)"
       : "=t" (res) : "0" (x), "u" (y) : "ax", "st(1)");
  return res;
}
