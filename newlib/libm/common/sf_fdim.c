/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "fdlibm.h"

	float fdimf(float x, float y)
{
  int c = __fpclassifyf(x);
  if (c == FP_NAN)  return(x);
  if (__fpclassifyf(y) == FP_NAN)  return(y);
  if (c == FP_INFINITE)
    return HUGE_VALF;

  return x > y ? x - y : 0.0;
}

#ifdef _DOUBLE_IS_32BITS

	double fdim(double x, double y)
{
  return (double) fdimf((float) x, (float) y);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
