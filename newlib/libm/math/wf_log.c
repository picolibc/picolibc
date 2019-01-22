/* wf_log.c -- float version of w_log.c.
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
 * wrapper logf(x)
 */

#include "fdlibm.h"
#if __OBSOLETE_MATH
#include <errno.h>

#ifdef __STDC__
	float logf(float x)		/* wrapper logf */
#else
	float logf(x)			/* wrapper logf */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_logf(x);
#else
	float z;
	z = __ieee754_logf(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x) || x > 0.0f) return z;
	if(x==0.0f) {
	    /* logf(0) */
	    errno = ERANGE;
	    return -HUGE_VALF;
	} else { 
	    /* logf(x<0) */
	    errno = EDOM;
	    return nanf("");
        }
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double log(double x)
#else
	double log(x)
	double x;
#endif
{
	return (double) logf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
#endif /* __OBSOLETE_MATH */
