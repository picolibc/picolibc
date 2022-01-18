#ifdef __SPE__

#include <_ansi.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <reent.h>
#include "fix64.h"

/*
 * Convert a string to a fixed-point (sign + 63-bits) value.
 *
 * Ignores `locale' stuff.
 */
__int64_t
_strtosfix64_r (struct _reent *rptr,
	const char *nptr,
	char **endptr)
{
  union long_double_union ldbl;
  int exp, negexp, sign, ld_type;
  __uint64_t tmp, tmp2;
  __int64_t result = 0;

  init(ldbl);

  _simdstrtold ((char *)nptr, endptr, &ldbl);

  /* treat NAN as domain error, +/- infinity as saturation */
  ld_type = _simdldcheck (&ldbl);
  if (ld_type != 0)
    {
      if (ld_type == 1)
	{
	  _REENT_ERRNO(rptr) = EDOM;
	  return 0;
	}
      _REENT_ERRNO(rptr) = ERANGE;
      if (word0(ldbl) & Sign_bit)
	return LONG_LONG_MIN;
      return LONG_LONG_MAX;
    }

  /* strip off sign and exponent */
  sign = word0(ldbl) & Sign_bit;
  exp = ((word0(ldbl) & Exp_mask) >> Exp_shift) - Bias;
  negexp = -exp;
  if (negexp > 63)
    return 0;
  word0(ldbl) &= ~(Exp_mask | Sign_bit);
  /* add in implicit normalized bit */
  word0(ldbl) |= Exp_msk1;
  /* shift so result is contained in single word */
  tmp = word0(ldbl) << Ebits;
  tmp |= ((unsigned long)word1(ldbl) >> (32 - Ebits));
  tmp <<= 32;
  if (Ebits < 32)
    tmp |= ((unsigned long)word1(ldbl) << Ebits);
  tmp |= ((unsigned long)word2(ldbl) >> (32 - Ebits));

  /* check for saturation */
  if (sign)
    {
      if (exp > 0 || (exp == 0 && tmp != 0x8000000000000000LL))
	{
	  _REENT_ERRNO(rptr) = ERANGE;
	  return LONG_LONG_MIN;
	}
    }
  else
    {
      if (exp >= 0)
	{
	  _REENT_ERRNO(rptr) = ERANGE;
	  return LONG_LONG_MAX;
	}
    }

  /* otherwise we have normal number in range */
  if (negexp != 0)
    {
      /* perform rounding */
      tmp2 = tmp + (1 << (negexp - 1));
      result = (long long)(tmp2 >> negexp);
      /* check if rounding caused carry bit which must be added into result */
      if (tmp2 < tmp)
	result |= (1 << (64 - negexp));
      /* check if positive saturation has occurred because of rounding */
      if (!sign && result < 0)
	{
	  _REENT_ERRNO(rptr) = ERANGE;
	  return LONG_LONG_MAX;
	}
    }
  else
    {
      /* we have -1.0, no rounding necessary */
      return LONG_LONG_MIN;
    }

  return sign ? -result : result;
}

#ifndef _REENT_ONLY

__int64_t
strtosfix64 (const char *s,
	char **ptr)
{
  return _strtosfix64_r (_REENT, s, ptr);
}

#endif

#endif /* __SPE__ */
