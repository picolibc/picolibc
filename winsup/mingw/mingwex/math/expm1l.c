/*
 * Written 2005 by Gregory W. Chicares <chicares@cox.net> with
 * help from Danny Smith. dannysmith@users.sourceforge.net>.
 * Public domain.
 *
 * F2XM1's input is constrained to (-1, +1), so the domain of
 * 'x * LOG2EL' is (-LOGE2L, +LOGE2L). Outside that domain,
 * delegating to expl() handles C99 7.12.6.3/2 range errors.
 *
 * Constants from moshier.net, file cephes/ldouble/constl.c,
 * are used instead of M_LN2 and M_LOG2E, which would not be
 * visible with 'gcc std=c99'.  The use of these extended precision
 * constants also allows gcc to replace them with x87 opcodes.
 */

#include <math.h> /* expl() */
#include "cephes_mconf.h"

long double expm1l (long double x)
{
  if (fabsl(x) < LOGE2L)
    {
      x *= LOG2EL;
      __asm__("f2xm1" : "=t" (x) : "0" (x));
      return x;
    }
  else
    return expl(x) - 1.0L;
}
