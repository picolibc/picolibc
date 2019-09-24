/* wf_fmod.c -- float version of w_fmod.c.
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
 * wrapper fmodf(x,y)
 */

#include "fdlibm.h"
#include <errno.h>

#if !defined(_IEEE_LIBM) || !defined(HAVE_ALIAS_ATTRIBUTE)
#ifdef __STDC__
	float fmodf(float x, float y)	/* wrapper fmodf */
#else
	float fmodf(x,y)		/* wrapper fmodf */
	float x,y;
#endif
{
	float z;
	z = __ieee754_fmodf(x,y);
	if(_LIB_VERSION == _IEEE_ ||isnan(y)||isnan(x)) return z;
	if(y==0.0f) {
            /* fmodf(x,0) */
	    errno = EDOM;
	    return  0.0f/0.0f;
	} else
	    return z;
}
#endif

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double fmod(double x, double y)
#else
	double fmod(x,y)
	double x,y;
#endif
{
	return (double) fmodf((float) x, (float) y);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
