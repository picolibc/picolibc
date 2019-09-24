/* wf_hypot.c -- float version of w_hypot.c.
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
 * wrapper hypotf(x,y)
 */

#include "fdlibm.h"
#include <errno.h>

#if !defined(_IEEE_LIBM) || !defined(HAVE_ALIAS_ATTRIBUTE)
#ifdef __STDC__
	float hypotf(float x, float y)	/* wrapper hypotf */
#else
	float hypotf(x,y)		/* wrapper hypotf */
	float x,y;
#endif
{
	float z;
	z = __ieee754_hypotf(x,y);
	if(_LIB_VERSION == _IEEE_) return z;
	if((!finitef(z))&&finitef(x)&&finitef(y)) {
	    /* hypotf(finite,finite) overflow */
	    errno = ERANGE;
	    return HUGE_VALF;
	} else
	    return z;
}
#endif

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double hypot(double x, double y)
#else
	double hypot(x,y)
	double x,y;
#endif
{
	return (double) hypotf((float) x, (float) y);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
