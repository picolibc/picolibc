/* @(#)w_gamma.c 5.1 93/09/24 */
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

/* double gamma(double x)
 * Return  the logarithm of the Gamma function of x or the Gamma function of x,
 * depending on the library mode.
 */

#include "fdlibm.h"
#include <errno.h>

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double tgamma(double x)
#else
	double tgamma(x)
	double x;
#endif
{
        double y;
	y = __ieee754_tgamma(x);
#ifdef _IEEE_LIBM
	return y;
#else
	if(_LIB_VERSION == _IEEE_) return y;

	if(!finite(y)) {
	  if(x < 0.0 && floor(x)==x)
	    errno = EDOM;
	  else if (finite(x))
	    errno = ERANGE;
	}
	return y;
#endif
}

#endif /* defined(_DOUBLE_IS_32BITS) */
