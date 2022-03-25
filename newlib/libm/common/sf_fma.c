/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "fdlibm.h"

#if !_HAVE_FAST_FMAF

#ifdef __STDC__
	float fmaf(float x, float y, float z)
#else
	float fmaf(x,y,z)
	float x;
	float y;
        float z;
#endif
{
  /*
   * Better to just get a double-rounded answer than invoke double
   * precision operations and get exceptions messed up
   */
  return x * y + z;
}

#endif

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
