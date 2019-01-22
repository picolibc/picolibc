/* wf_j1.c -- float version of w_j1.c.
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
 * wrapper of j1f,y1f 
 */

#include "fdlibm.h"
#include <errno.h>


#ifdef __STDC__
	float j1f(float x)		/* wrapper j1f */
#else
	float j1f(x)			/* wrapper j1f */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_j1f(x);
#else
	float z;
	z = __ieee754_j1f(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x) ) return z;
	if(fabsf(x)>(float)X_TLOSS) {
	    /* j1f(|x|>X_TLOSS) */
	    errno = ERANGE;
	    return 0.0f;
	} else
	    return z;
#endif
}

#ifdef __STDC__
	float y1f(float x)		/* wrapper y1f */
#else
	float y1f(x)			/* wrapper y1f */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_y1f(x);
#else
	float z;
	z = __ieee754_y1f(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x) ) return z;
        if(x <= 0.0f){
	    /* y1f(0) = -inf or y1f(x<0) = NaN */
	    errno = EDOM;
	    return -HUGE_VALF;
        }
	if(x>(float)X_TLOSS) {
	    /* y1f(x>X_TLOSS) */
	    errno = ERANGE;
	    return 0.0f;
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double j1(double x)
#else
	double j1(x)
	double x;
#endif
{
	return (double) j1f((float) x);
}

#ifdef __STDC__
	double y1(double x)
#else
	double y1(x)
	double x;
#endif
{
	return (double) y1f((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
