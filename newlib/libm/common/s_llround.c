/* lround adapted to be llround for Newlib, 2009 by Craig Howland.  */
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

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

long long int
llround64(__float64 x)
{
  __int32_t sign, exponent_less_1023;
  /* Most significant word, least significant word. */
  __uint32_t msw, lsw;
  long long int result;
  
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
  else if (exponent_less_1023 < (__int32_t) ((8 * sizeof (long long int)) - 1))
    {
      /* 64bit longlong: exponent_less_1023 in [20,62] */
      if (exponent_less_1023 >= 52)
	/* 64bit longlong: exponent_less_1023 in [52,62] */
	/* 64bit longlong: shift amt in [32,42] */
        result = ((long long int) msw << (exponent_less_1023 - 20))
		    /* 64bit longlong: shift amt in [0,10] */
                    | (lsw << (exponent_less_1023 - 52));
      else
        {
	  /* 64bit longlong: exponent_less_1023 in [20,51] */
          unsigned int tmp = lsw
		    /* 64bit longlong: shift amt in [0,31] */
                    + (0x80000000 >> (exponent_less_1023 - 20));
          if (tmp < lsw)
            ++msw;
	  /* 64bit longlong: shift amt in [0,31] */
          result = ((long long int) msw << (exponent_less_1023 - 20))
		    /* ***64bit longlong: shift amt in [32,1] */
                    | SAFE_RIGHT_SHIFT (tmp, (52 - exponent_less_1023));
        }
    }
  else
    /* Result is too large to be represented by a long long int. */
    return (long long int)x;

  return sign * result;
}

_MATH_ALIAS_k_d(llround)

#endif /* _NEED_FLOAT64 */
