#include <math.h>
#include <errno.h>
#include "fastmath.h"

/* acosh(x) = log (x + sqrt(x * x - 1)) */
long double acoshl (long double x)
{
  if (isnan (x)) 
    return x;

  if (x < 1.0L)
    {
      errno = EDOM;
      return nanl("");
    }
  if (x > 0x1p32L)
    /* Avoid overflow (and unnecessary calculation when
       sqrt (x * x - 1) == x).
       The M_LN2 define doesn't have enough precison for
       long double so use this one. GCC optimizes by replacing
       the const with a fldln2 insn. */
    return __fast_logl (x) + 6.9314718055994530941723E-1L;

   /* Since  x >= 1, the arg to log will always be greater than
      the fyl2xp1 limit (approx 0.29) so just use logl. */ 
   return __fast_logl (x + __fast_sqrtl((x + 1.0L) * (x - 1.0L)));
}
