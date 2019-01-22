/* wf_cosh.c -- float version of w_cosh.c.
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
 * wrapper coshf(x)
 */

#include "fdlibm.h"
#include <errno.h>

#ifdef __STDC__
	float coshf(float x)		/* wrapper coshf */
#else
	float coshf(x)			/* wrapper coshf */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_coshf(x);
#else
	float z;
	z = __ieee754_coshf(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(fabsf(x)>8.9415985107e+01f) {
	    /* coshf(finite) overflow */
	    errno = ERANGE;
	    return HUGE_VALF;
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double cosh(double x)
#else
	double cosh(x)
	double x;
#endif
{
	return (double) coshf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
