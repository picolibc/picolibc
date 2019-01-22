/* wf_scalb.c -- float version of w_scalb.c.
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
 * wrapper scalbf(float x, float fn) is provide for
 * passing various standard test suite. One 
 * should use scalbn() instead.
 */

#include "fdlibm.h"
#include <errno.h>

#ifdef __STDC__
#ifdef _SCALB_INT
	float scalbf(float x, int fn)		/* wrapper scalbf */
#else
	float scalbf(float x, float fn)		/* wrapper scalbf */
#endif
#else
	float scalbf(x,fn)			/* wrapper scalbf */
#ifdef _SCALB_INT
	float x; int fn;
#else
	float x,fn;
#endif
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_scalbf(x,fn);
#else
	float z;
	z = __ieee754_scalbf(x,fn);
	if(_LIB_VERSION == _IEEE_) return z;
	if(!(finitef(z)||isnan(z))&&finitef(x)) {
	    /* scalbf overflow; */
	    errno = ERANGE;
	    return (x > 0.0 ? HUGE_VALF : -HUGE_VALF);
	}
	if(z==0.0f&&z!=x) {
	    /* scalbf underflow */
	    errno = ERANGE;
	    return copysign(0.0,x);
	} 
#ifndef _SCALB_INT
	if(!finitef(fn)) errno = ERANGE;
#endif
	return z;
#endif 
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
#ifdef _SCALB_INT
	double scalb(double x, int fn)
#else 
	double scalb(double x, double fn)
#endif
#else
	double scalb(x, fn)
#ifdef _SCALB_INT
	double x; int fn;
#else
	double x,fn;
#endif
#endif
{
#ifdef _SCALB_INT
	return (double) scalbf((float) x, fn);
#else
	return (double) scalbf((float) x, (float) fn);
#endif
}

#endif /* defined(_DOUBLE_IS_32BITS) */
