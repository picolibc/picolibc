/* wf_j0.c -- float version of w_j0.c.
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
 * wrapper j0f(float x), y0f(float x)
 */

#include "fdlibm.h"
#include <errno.h>

#if !defined(_IEEE_LIBM) || !defined(HAVE_ALIAS_ATTRIBUTE)
#ifdef __STDC__
	float j0f(float x)		/* wrapper j0f */
#else
	float j0f(x)			/* wrapper j0f */
	float x;
#endif
{
	float z = __ieee754_j0f(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(fabsf(x)>(float)X_TLOSS) {
	    /* j0f(|x|>X_TLOSS) */
	    errno = ERANGE;
	}
	return z;
}
#endif

#if !defined(_IEEE_LIBM) || !defined(HAVE_ALIAS_ATTRIBUTE)
#ifdef __STDC__
	float y0f(float x)		/* wrapper y0f */
#else
	float y0f(x)			/* wrapper y0f */
	float x;
#endif
{
	float z;
	z = __ieee754_y0f(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x) ) return z;
        if(x < 0.0f){
	    /* y0f(x<0) = NaN */
	    errno = EDOM;
        }
	if (x == 0.0f){
	    /* y0f(n,0) = -inf */
	    errno = ERANGE;
	}
	if(x>(float)X_TLOSS) {
	    /* y0f(x>X_TLOSS) */
	    errno = ERANGE;
	}
	return z;
}
#endif

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double j0(double x)
#else
	double j0(x)
	double x;
#endif
{
	return (double) j0f((float) x);
}

#ifdef __STDC__
	double y0(double x)
#else
	double y0(x)
	double x;
#endif
{
	return (double) y0f((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
