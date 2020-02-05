/*
Copyright (c) 1984,2000 S.L. Moshier

Permission to use, copy, modify, and distribute this software for any
purpose without fee is hereby granted, provided that this entire notice
is included in all copies of any software which is or includes a copy
or modification of this software and in all copies of the supporting
documentation for such software.

THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
WARRANTY.  IN PARTICULAR,  THE AUTHOR MAKES NO REPRESENTATION
OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */
#ifdef __SPE__

#include <_ansi.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <reent.h>
#include "vfieeefp.h"

/*
 * Convert a string to a fixed-point (sign + 31-bits) value.
 *
 * Ignores `locale' stuff.
 */
__int32_t
_strtosfix32_r (struct _reent *rptr,
	const char *nptr,
	char **endptr)
{
  union double_union dbl;
  int exp, negexp, sign;
  unsigned long tmp, tmp2;
  long result = 0;

  dbl.d = _strtod_r (rptr, nptr, endptr);

  /* treat NAN as domain error, +/- infinity as saturation */
  if (!finite(dbl.d))
    {
      if (isnan (dbl.d))
	{
	  __errno_r(rptr) = EDOM;
	  return 0;
	}
      __errno_r(rptr) = ERANGE;
      if (word0(dbl) & Sign_bit)
	return LONG_MIN;
      return LONG_MAX;
    }

  /* check for normal saturation */
  if (dbl.d >= 1.0)
    {
      __errno_r(rptr) = ERANGE;
      return LONG_MAX;
    }
  else if (dbl.d < -1.0)
    {
      __errno_r(rptr) = ERANGE;
      return LONG_MIN;
    }

  /* otherwise we have normal number in range */

  /* strip off sign and exponent */
  sign = word0(dbl) & Sign_bit;
  exp = ((word0(dbl) & Exp_mask) >> Exp_shift) - Bias;
  negexp = -exp;
  if (negexp > 31)
    return 0;
  word0(dbl) &= ~(Exp_mask | Sign_bit);
  /* add in implicit normalized bit */
  word0(dbl) |= Exp_msk1;
  /* shift so result is contained in single word */
  tmp = word0(dbl) << Ebits;
  tmp |= ((unsigned long)word1(dbl) >> (32 - Ebits));
  if (negexp != 0)
    {
      /* perform rounding */
      tmp2 = tmp + (1 << (negexp - 1));
      result = (long)(tmp2 >> negexp);
      /* check if rounding caused carry bit which must be added into result */
      if (tmp2 < tmp)
	result |= (1 << (32 - negexp));
      /* check if positive saturation has occurred because of rounding */
      if (!sign && result < 0)
	{
	  __errno_r(rptr) = ERANGE;
	  return LONG_MAX;
	}
    }
  else
    {
      /* we have -1.0, no rounding necessary */
      return LONG_MIN;
    }

  return sign ? -result : result;
}

#ifndef _REENT_ONLY

__int32_t
strtosfix32 (const char *s,
	char **ptr)
{
  return _strtosfix32_r (_REENT, s, ptr);
}

#endif

#endif /* __SPE__ */
