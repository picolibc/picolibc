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
<<round>>, <<roundf>>---round to integer, to nearest
INDEX
	round
INDEX
	roundf

ANSI_SYNOPSIS
	#include <math.h>
	double round(double <[x]>);
	float roundf(float <[x]>);

DESCRIPTION
	The <<round>> functions round their argument to the nearest integer
	value in floating-point format, rounding halfway cases away from zero,
	regardless of the current rounding direction.  (While the "inexact"
	floating-point exception behavior is unspecified by the C standard, the
	<<round>> functions are written so that "inexact" is not raised if the
	result does not equal the argument, which behavior is as recommended by
	IEEE 754 for its related functions.)

RETURNS
<[x]> rounded to an integral value.

PORTABILITY
ANSI C, POSIX

SEEALSO
<<nearbyint>>, <<rint>>

*/

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double round(double x)
#else
	double round(x)
	double x;
#endif
{
  /* Most significant word, least significant word. */
  __int32_t msw, exponent_less_1023;
  __uint32_t lsw;

  EXTRACT_WORDS(msw, lsw, x);

  /* Extract exponent field. */
  exponent_less_1023 = ((msw & 0x7ff00000) >> 20) - 1023;

  if (exponent_less_1023 < 20)
    {
      if (exponent_less_1023 < 0)
        {
          msw &= 0x80000000;
          if (exponent_less_1023 == -1)
            /* Result is +1.0 or -1.0. */
            msw |= (1023 << 20);
          lsw = 0;
        }
      else
        {
          __uint32_t exponent_mask = 0x000fffff >> exponent_less_1023;
          if ((msw & exponent_mask) == 0 && lsw == 0)
            /* x in an integral value. */
            return x;

          msw += 0x00080000 >> exponent_less_1023;
          msw &= ~exponent_mask;
          lsw = 0;
        }
    }
  else if (exponent_less_1023 > 51)
    {
      if (exponent_less_1023 == 1024)
        /* x is NaN or infinite. */
        return x + x;
      else
        return x;
    }
  else
    {
      __uint32_t exponent_mask = 0xffffffff >> (exponent_less_1023 - 20);
      __uint32_t tmp;

      if ((lsw & exponent_mask) == 0)
        /* x is an integral value. */
        return x;

      tmp = lsw + (1 << (51 - exponent_less_1023));
      if (tmp < lsw)
        msw += 1;
      lsw = tmp;

      lsw &= ~exponent_mask;
    }
  INSERT_WORDS(x, msw, lsw);

  return x;
}

#endif /* _DOUBLE_IS_32BITS */
