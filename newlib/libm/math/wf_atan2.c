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

#ifdef __STDC__
	float atan2f(float y, float x)		/* wrapper atan2f */
#else
	float atan2f(y,x)			/* wrapper atan2 */
	float y,x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_atan2f(y,x);
#else
	float z;
	struct exception exc;
	z = __ieee754_atan2f(y,x);
	if(_LIB_VERSION == _IEEE_||isnanf(x)||isnanf(y)) return z;
	if(x==(float)0.0&&y==(float)0.0) {
	    /* atan2f(+-0,+-0) */
	    exc.arg1 = y;
	    exc.arg2 = x;
	    exc.err = 0;
	    exc.type = DOMAIN;
	    exc.name = "atan2f";
	    exc.retval = 0.0;
	    if(_LIB_VERSION == _POSIX_)
	       errno = EDOM;
	    else if (!matherr(&exc)) {
	       errno = EDOM;
	    }
	    if (exc.err != 0)
	       errno = exc.err;
	    return (float)exc.retval;
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double atan2(double y, double x)
#else
	double atan2(y,x)
	double y,x;
#endif
{
	return (double) atan2f((float) y, (float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
