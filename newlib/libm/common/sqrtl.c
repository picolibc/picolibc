/*
(C) Copyright IBM Corp. 2009

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of IBM nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <math.h>
#include "local.h"

#ifdef _LDBL_EQ_DBL
/* On platforms where long double is as wide as double.  */
long double
sqrtl (long double x)
{
  return sqrt(x);
}

#else

  /* This code is based upon the version in the BSD math's library.
     That code is...
   *
   * Copyright (c) 2007 Steven G. Kargl
   * All rights reserved.
   *
   * Redistribution and use in source and binary forms, with or without
   * modification, are permitted provided that the following conditions
   * are met:
   * 1. Redistributions of source code must retain the above copyright
   *    notice unmodified, this list of conditions, and the following
   *    disclaimer.
   * 2. Redistributions in binary form must reproduce the above copyright
   *    notice, this list of conditions and the following disclaimer in the
   *    documentation and/or other materials provided with the distribution.
   *
   * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   */
#include <float.h>
#include "ieeefp.h"

#ifndef LDBL_NBIT
#define	LDBL_NBIT	0
#endif

#ifndef LDBL_MAX_EXP
#define LDBL_MAX_EXP	DBL_MAX_EXP
#endif

/* Return (x + ulp) for normal positive x.  Assumes no overflow.  */

static inline long double
inc (long double x)
{
  union ieee_ext_u ux = { .extu_ld = x, };

  if (++ux.extu_ext.ext_fracl == 0)
    {
      if (++ux.extu_ext.ext_frach == 0)
	{
	  ux.extu_ext.ext_exp++;
	  ux.extu_ext.ext_frach |= LDBL_NBIT;
	}
    }

  return ux.extu_ld;
}

/* Return (x - ulp) for normal positive x.  Assumes no underflow.  */

static inline long double
dec (long double x)
{
  union ieee_ext_u ux = { .extu_ld = x, };

  if (ux.extu_ext.ext_fracl-- == 0)
    {
      if (ux.extu_ext.ext_frach-- == LDBL_NBIT)
	{
	  ux.extu_ext.ext_exp--;
	  ux.extu_ext.ext_frach |= LDBL_NBIT;
	}
    }

  return ux.extu_ld;
}

/* This is slow, but simple and portable.  */

long double
sqrtl (long double x)
{
  union ieee_ext_u ux = { .extu_ld = x, };
  int k, r;
  long double lo, xn;

  /* If x = NaN, then sqrt(x) = NaN.  */
  /* If x = Inf, then sqrt(x) = Inf.  */
  /* If x = -Inf, then sqrt(x) = NaN.  */
  if (ux.extu_ext.ext_exp == LDBL_MAX_EXP * 2 - 1)
    return (x * x + x);

  /* If x = +-0, then sqrt(x) = +-0.  */
  if (x == 0.0L || x == -0.0L)
    return x;

  /* If x < 0, then raise invalid and return NaN.  */
  if (ux.extu_ext.ext_sign)
    return ((x - x) / (x - x));

  if (ux.extu_ext.ext_exp == 0)
    {
      /* Adjust subnormal numbers.  */
      ux.extu_ld *= 0x1.0p514;
      k = -514;
    }
  else
    k = 0;

  /* ux.extu_ld is a normal number, so break it into ux.extu_ld = e*2^n where
     ux.extu_ld = (2*e)*2^2k for odd n and ux.extu_ld = (4*e)*2^2k for even n.  */

  if ((ux.extu_ext.ext_exp - EXT_EXP_BIAS) & 1)
    {
      /* n is even.  */
      k += ux.extu_ext.ext_exp - EXT_EXP_BIAS - 1; /* 2k = n - 2.  */
      ux.extu_ext.ext_exp = EXT_EXP_BIAS + 1;	   /* ux.extu_ld in [2,4).  */
    }
  else
    {
      k += ux.extu_ext.ext_exp - EXT_EXP_BIAS;	/* 2k = n - 1.  */
      ux.extu_ext.ext_exp = EXT_EXP_BIAS;	/* ux.extu_ld in [1,2).  */
    }

  /* Newton's iteration.
     Split ux.extu_ld into a high and low part to achieve additional precision.  */

  xn = sqrt ((double) ux.extu_ld);	/* 53-bit estimate of sqrtl(x).  */

#if LDBL_MANT_DIG > 100
  xn = (xn + (ux.extu_ld / xn)) * 0.5;	/* 106-bit estimate.  */
#endif

  lo = ux.extu_ld;
  ux.extu_ext.ext_fracl = 0;		/* Zero out lower bits.  */
  lo = (lo - ux.extu_ld) / xn;		/* Low bits divided by xn.  */
  xn = xn + (ux.extu_ld / xn);		/* High portion of estimate.  */
  ux.extu_ld = xn + lo;			/* Combine everything.  */
  ux.extu_ext.ext_exp += (k >> 1) - 1;

  xn = x / ux.extu_ld;			/* Chopped quotient (inexact?).  */

  /* For simplicity we round to nearest.  */
  xn = inc (xn);			/* xn = xn + ulp.  */

  ux.extu_ld = ux.extu_ld + xn;		/* Chopped sum.  */
  ux.extu_ext.ext_exp--;

  return ux.extu_ld;
}
#endif /* ! _LDBL_EQ_DBL */


