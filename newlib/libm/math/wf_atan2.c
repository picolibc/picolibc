/* wf_atan2.c -- float version of w_atan2.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

/* 
 * wrapper atan2f(y,x)
 */

#include "fdlibm.h"
#include <errno.h>

	float atan2f(float y, float x)		/* wrapper atan2f */
{
	return __ieee754_atan2f(y,x);
}

#ifdef _DOUBLE_IS_32BITS

	double atan2(double y, double x)
{
	return (double) atan2f((float) y, (float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
