/* wf_gamma.c -- float version of w_gamma.c.
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

#include "fdlibm.h"
#include <reent.h>
#include <errno.h>

#ifdef __STDC__
	float gammaf(float x)
#else
	float gammaf(x)
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_gammaf_r(x,&(_REENT_SIGNGAM(_REENT)));
#else
        float y;
        y = __ieee754_gammaf_r(x,&(_REENT_SIGNGAM(_REENT)));
        if(_LIB_VERSION == _IEEE_) return y;
        if(!finitef(y)&&finitef(x)) {
	    if(floorf(x)==x&&x<=0.0f) {
		/* gammaf(-integer) or gammaf(0) */
		errno = EDOM;
            } else {
		/* gammaf(finite) overflow */
		errno = ERANGE;
            }
	    return HUGE_VALF;
        } else
            return y;
#endif
}             

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double gamma(double x)
#else
	double gamma(x)
	double x;
#endif
{
	return (double) gammaf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
