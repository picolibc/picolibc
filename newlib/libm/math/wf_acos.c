/* wf_acos.c -- float version of w_acos.c.
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
 * wrap_acosf(x)
 */

#include "fdlibm.h"
#include <errno.h>

	float acosf(float x)		/* wrapper acosf */
{
#ifdef _IEEE_LIBM
	return __ieee754_acosf(x);
#else
	float z;
	z = __ieee754_acosf(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(fabsf(x)>1.0f) {
	    /* acosf(|x|>1) */
	    errno = EDOM;
	    return nanf("");
	} else
	    return z;
#endif
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double acos(double x)
#else
	double acos(x)
	double x;
#endif
{
	return (double) acosf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
