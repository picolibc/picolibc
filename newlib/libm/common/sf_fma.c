/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "fdlibm.h"

#ifdef __STDC__
	float fmaf(float x, float y, float z)
#else
	float fmaf(x,y,z)
	float x;
	float y;
        float z;
#endif
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

#ifdef __STDC__
	double fma(double x, double y, double z)
#else
	double fma(x,y,z)
	double x;
	double y;
        double z;
#endif
{
  return (double) fmaf((float) x, (float) y, (float) z);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
