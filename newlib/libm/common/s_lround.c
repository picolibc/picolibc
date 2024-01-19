/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */
/*
FUNCTION
<<lround>>, <<lroundf>>, <<llround>>, <<llroundf>>---round to integer, to nearest
INDEX
	lround
INDEX
	lroundf
INDEX
	llround
INDEX
	llroundf

SYNOPSIS
	#include <math.h>
	long int lround(double <[x]>);
	long int lroundf(float <[x]>);
	long long int llround(double <[x]>);
	long long int llroundf(float <[x]>);

DESCRIPTION
	The <<lround>> and <<llround>> functions round their argument to the
	nearest integer value, rounding halfway cases away from zero, regardless
	of the current rounding direction.  If the rounded value is outside the
	range of the return type, the numeric result is unspecified (depending
	upon the floating-point implementation, not the library).  A range
	error may occur if the magnitude of x is too large.

RETURNS
<[x]> rounded to an integral value as an integer.

SEEALSO
See the <<round>> functions for the return being the same floating-point type
as the argument.  <<lrint>>, <<llrint>>.

PORTABILITY
ANSI C, POSIX

*/

#include "fdlibm.h"
#include <limits.h>

#ifdef _NEED_FLOAT64

long int
lround64(__float64 x)
{
  __int32_t sign, exponent_less_1023;
  /* Most significant word, least significant word. */
  __uint32_t msw, lsw;
  long int result;

  EXTRACT_WORDS(msw, lsw, x);

  /* Extract sign. */
  sign = ((msw & 0x80000000) ? -1 : 1);
  /* Extract exponent field. */
  exponent_less_1023 = ((msw & 0x7ff00000) >> 20) - 1023;
  msw &= 0x000fffff;
  msw |= 0x00100000;
  /* exponent_less_1023 in [-1023,1024] */
  if (exponent_less_1023 < 20)
    {
      /* exponent_less_1023 in [-1023,19] */
      if (exponent_less_1023 < 0)
        {
          if (exponent_less_1023 < -1)
            return 0;
          else
            return sign;
        }
      else
        {
          /* exponent_less_1023 in [0,19] */
	  /* shift amt in [0,19] */
          msw += 0x80000 >> exponent_less_1023;
	  /* shift amt in [20,1] */
          result = msw >> (20 - exponent_less_1023);
        }
    }
  else if (exponent_less_1023 < (__int32_t) ((8 * sizeof (long int)) - 1))
    {
      /* 32bit long: exponent_less_1023 in [20,30] */
      /* 64bit long: exponent_less_1023 in [20,62] */
      if (exponent_less_1023 >= 52)
	/* 64bit long: exponent_less_1023 in [52,62] */
	/* 64bit long: shift amt in [32,42] */
        result = ((long int) msw << (exponent_less_1023 - 20))
		/* 64bit long: shift amt in [0,10] */
            | ((long int) lsw << (exponent_less_1023 - 52));
      else
        {
	  /* 32bit long: exponent_less_1023 in [20,30] */
	  /* 64bit long: exponent_less_1023 in [20,51] */
          __uint32_t tmp = lsw
		    /* 32bit long: shift amt in [0,10] */
		    /* 64bit long: shift amt in [0,31] */
                    + (0x80000000 >> (exponent_less_1023 - 20));
          if (tmp < lsw)
            ++msw;
	  /* 32bit long: shift amt in [0,10] */
	  /* 64bit long: shift amt in [0,31] */
          result = ((long int) msw << (exponent_less_1023 - 20))
		    /* ***32bit long: shift amt in [32,22] */
		    /* ***64bit long: shift amt in [32,1] */
                    | SAFE_RIGHT_SHIFT (tmp, (52 - exponent_less_1023));
        }
    }
  else
  {
    /* Result is too large to be represented by a long int. */
    if (sign == 1 ||
        !((sizeof(long) == 4 && x > LONG_MIN - _F_64(0.5)) ||
          (sizeof(long) > 4 && x >= LONG_MIN)))
    {
      __math_set_invalid();
      return sign == 1 ? LONG_MAX : LONG_MIN;
    }
    return (long int)x;
  }

  if (sizeof (long) == 4 && sign == 1 && result == LONG_MIN)
    __math_set_invalid();

  return sign * result;
}

_MATH_ALIAS_j_d(lround)

#endif /* _NEED_FLOAT64 */
