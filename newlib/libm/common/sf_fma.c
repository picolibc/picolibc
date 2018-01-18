/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "fdlibm.h"

	float fmaf(float x, float y, float z)
{
  /* NOTE:  The floating-point exception behavior of this is not as
   * required.  But since the basic function is not really done properly,
   * it is not worth bothering to get the exceptions right, either.  */
  /* Let the implementation handle this. */ /* <= NONSENSE! */
  /* In floating-point implementations in which double is larger than float,
   * computing as double should provide the desired function.  Otherwise,
   * the behavior will not be as specified in the standards.  */
  return (float) (((double) x * (double) y) + (double) z);
}

#ifdef _DOUBLE_IS_32BITS

	double fma(double x, double y, double z)
{
  return (double) fmaf((float) x, (float) y, (float) z);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
