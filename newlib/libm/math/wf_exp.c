/* wf_exp.c -- float version of w_exp.c.
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
 * wrapper expf(x)
 */

#include "fdlibm.h"
#if __OBSOLETE_MATH
#include <errno.h>

#ifdef __STDC__
static const float
#else
static float
#endif
o_threshold=  0x1.62e42ep+06,  /* 0x42b17217 */
u_threshold= -0x1.9fe36ap+06;  /* 0xc2cff1b5 */

#ifdef __STDC__
	float expf(float x)		/* wrapper expf */
#else
	float expf(x)			/* wrapper expf */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_expf(x);
#else
	float z;
	z = __ieee754_expf(x);
	if(_LIB_VERSION == _IEEE_) return z;
	if(finitef(x)) {
	    if(x>o_threshold) {
		/* expf(finite) overflow */
		errno = ERANGE;
	        return HUGE_VALF;
	    } else if(x<u_threshold) {
		/* expf(finite) underflow */
		errno = ERANGE;
		return 0.0f;
	    } 
	} 
	return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double exp(double x)
#else
	double exp(x)
	double x;
#endif
{
	return (double) expf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
#endif /* __OBSOLETE_MATH */
