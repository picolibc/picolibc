/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "fdlibm.h"

#ifdef __STDC__
	float fdimf(float x, float y)
#else
	float fdimf(x,y)
	float x;
	float y;
#endif
{
  if (isnanf(x) || isnanf(y)) return(x+y);

  float z = x > y ? x - y : 0.0f;
  if (!isinf(x) && !isinf(y))
    z = check_oflowf(z);
  return z;
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double fdim(double x, double y)
#else
	double fdim(x,y)
	double x;
	double y;
#endif
{
  return (double) fdimf((float) x, (float) y);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
