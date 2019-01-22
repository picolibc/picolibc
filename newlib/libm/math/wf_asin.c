/* wf_asin.c -- float version of w_asin.c.
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
 * wrapper asinf(x)
 */


#include "fdlibm.h"
#include <errno.h>

#ifdef __STDC__
	float asinf(float x)		/* wrapper asinf */
#else
	float asinf(x)			/* wrapper asinf */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_asinf(x);
#else
	float z;
	z = __ieee754_asinf(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(fabsf(x)>1.0f) {
	    /* asinf(|x|>1) */
	    errno = EDOM;
	    return nanf("");
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double asin(double x)
#else
	double asin(x)
	double x;
#endif
{
	return (double) asinf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
