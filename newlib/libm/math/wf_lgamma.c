/* wf_lgamma.c -- float version of w_lgamma.c.
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
	float lgammaf(float x)
#else
	float lgammaf(x)
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_lgammaf_r(x,&(_REENT_SIGNGAM(_REENT)));
#else
        float y;
        y = __ieee754_lgammaf_r(x,&(_REENT_SIGNGAM(_REENT)));
        if(_LIB_VERSION == _IEEE_) return y;
        if(!finitef(y)&&finitef(x)) {
	    if(floorf(x)==x&&x<=0.0f) {
		/* lgammaf(-integer) */
		errno = EDOM;
	    } else {
		/* lgammaf(finite) overflow */
		errno = ERANGE;
	    }
            return HUGE_VALF;
        } else
            return y;
#endif
}             

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double lgamma(double x)
#else
	double lgamma(x)
	double x;
#endif
{
	return (double) lgammaf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
