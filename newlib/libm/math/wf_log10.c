/* wf_log10.c -- float version of w_log10.c.
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
 * wrapper log10f(X)
 */

#include "fdlibm.h"
#include <errno.h>

#ifdef __STDC__
	float log10f(float x)		/* wrapper log10f */
#else
	float log10f(x)			/* wrapper log10f */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_log10f(x);
#else
	float z;
	z = __ieee754_log10f(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(x<=0.0f) {
	    if(x==0.0f) {
		/* log10f(0) */
		errno = ERANGE;
		return -HUGE_VALF;
	    } else { 
		/* log10f(x<0) */
		errno = EDOM;
		return nanf("");
            }
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double log10(double x)
#else
	double log10(x)
	double x;
#endif
{
	return (double) log10f((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
