/* sf_isnan.c -- float version of s_isnan.c.
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
 */

/*
 * isnanf(x) returns 1 is x is nan, else 0;
 * no branching!
 */

#include "fdlibm.h"

#ifdef __STDC__
	int isnanf(float x)
#else
	int isnanf(x)
	float x;
#endif
{
	__int32_t ix;
	GET_FLOAT_WORD(ix,x);
	ix &= 0x7fffffff;
	ix = 0x7f800000 - ix;
	return (int)(((__uint32_t)(ix))>>31);
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	int isnan(double x)
#else
	int isnan(x)
	double x;
#endif
{
	return isnanf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
