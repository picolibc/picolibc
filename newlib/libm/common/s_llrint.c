/* lrint adapted to be llrint for Newlib, 2009 by Craig Howland.  */
/* @(#)s_lrint.c 5.1 93/09/24 */
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
 * llrint(x)
 * Return x rounded to integral value according to the prevailing
 * rounding mode.
 * Method:
 *	Using floating addition.
 * Exception:
 *	Inexact flag raised if x not equal to llrint(x).
 */

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
static const double
#else
static double 
#endif

/* Adding a double, x, to 2^52 will cause the result to be rounded based on
   the fractional part of x, according to the implementation's current rounding
   mode.  2^52 is the smallest double that can be represented using all 52 significant
   digits. */
TWO52[2]={
  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
 -4.50359962737049600000e+15, /* 0xC3300000, 0x00000000 */
};

long long int
#ifdef __STDC__
	llrint(double x)
#else
	llrint(x)
	double x;
#endif
{
  __int32_t i0,j0,sx;
  __uint32_t i1;
  double t;
  volatile double w;
  long long int result;
  
  EXTRACT_WORDS(i0,i1,x);

  /* Extract sign bit. */
  sx = (i0>>31)&1;

  /* Extract exponent field. */
  j0 = ((i0 & 0x7ff00000) >> 20) - 1023;
  /* j0 in [-1023,1024] */
  
  if(j0 < 20)
    {
      /* j0 in [-1023,19] */
      if(j0 < -1)
        return 0;
      else
        {
          /* j0 in [0,19] */
	  /* shift amt in [0,19] */
          w = TWO52[sx] + x;
          t = w - TWO52[sx];
          GET_HIGH_WORD(i0, t);
          /* Detect the all-zeros representation of plus and
             minus zero, which fails the calculation below. */
          if ((i0 & ~((__int32_t)1 << 31)) == 0)
              return 0;
          /* After round:  j0 in [0,20] */
          j0 = ((i0 & 0x7ff00000) >> 20) - 1023;
          i0 &= 0x000fffff;
          i0 |= 0x00100000;
	  /* shift amt in [20,0] */
          result = i0 >> (20 - j0);
        }
    }
  else if (j0 < (int)(8 * sizeof (long long int)) - 1)
    {
      /* 64bit return: j0 in [20,62] */
      if (j0 >= 52)
	/* 64bit return: j0 in [52,62] */
	/* 64bit return: left shift amt in [32,42] */
        result = ((long long int) ((i0 & 0x000fffff) | 0x0010000) << (j0 - 20)) | 
		/* 64bit return: right shift amt in [0,10] */
                   (i1 << (j0 - 52));
      else
        {
	  /* 64bit return: j0 in [20,51] */
          w = TWO52[sx] + x;
          t = w - TWO52[sx];
          EXTRACT_WORDS (i0, i1, t);
          j0 = ((i0 & 0x7ff00000) >> 20) - 1023;
          i0 &= 0x000fffff;
          i0 |= 0x00100000;
          /* After round:
	   * 64bit return: j0 in [20,52] */
	  /* 64bit return: left shift amt in [0,32] */
          /* ***64bit return: right shift amt in [32,0] */
          result = ((long long int) i0 << (j0 - 20))
			| SAFE_RIGHT_SHIFT (i1, (52 - j0));
        }
    }
  else
    {
      return (long long int) x;
    }
  
  return sx ? -result : result;
}

#endif /* _DOUBLE_IS_32BITS */
