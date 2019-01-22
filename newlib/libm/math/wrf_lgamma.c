/* wrf_lgamma.c -- float version of wr_lgamma.c.
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
 * wrapper float lgammaf_r(float x, int *signgamp)
 */

#include "fdlibm.h"
#include <errno.h>

#ifdef __STDC__
	float lgammaf_r(float x, int *signgamp) /* wrapper lgammaf_r */
#else
	float lgammaf_r(x,signgamp)              /* wrapper lgammaf_r */
	float x; int *signgamp;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_lgammaf_r(x,signgamp);
#else
        float y;
        y = __ieee754_lgammaf_r(x,signgamp);
        if(_LIB_VERSION == _IEEE_) return y;
        if(!finitef(y)&&finitef(x)) {
	    if(floorf(x)==x&&x<=0.0f) {
		/* lgammaf(-integer) or lgamma(0) */
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
