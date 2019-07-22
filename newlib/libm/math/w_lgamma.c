
/* @(#)w_lgamma.c 5.1 93/09/24 */
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

/* double lgamma(double x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call __ieee754_lgamma_r
 */

#include "fdlibm.h"
#include <reent.h>
#include <errno.h>

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double lgamma(double x)
#else
	double lgamma(x)
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_lgamma_r(x,&(_REENT_SIGNGAM(_REENT)));
#else
        double y;
        y = __ieee754_lgamma_r(x,&(_REENT_SIGNGAM(_REENT)));
        if(_LIB_VERSION == _IEEE_) return y;
        if(!finite(y)&&finite(x)) {
	    if(floor(x)==x&&x<=0.0)
	      /* lgamma(-integer) */
	      errno = EDOM;
	    else
	      /* lgamma(finite) overflow */
	      errno = ERANGE;
	    return HUGE_VAL;
	} else
            return y;
#endif
}             

#endif /* defined(_DOUBLE_IS_32BITS) */







