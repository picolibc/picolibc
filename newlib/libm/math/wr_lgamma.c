
/* @(#)wr_lgamma.c 5.1 93/09/24 */
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
 * wrapper double lgamma_r(double x, int *signgamp)
 */

#include "fdlibm.h"
#include <errno.h>

#ifndef _DOUBLE_IS_32BITS

#if !defined(_IEEE_LIBM) || !defined(HAVE_ALIAS_ATTRIBUTE)
#ifdef __STDC__
	double lgamma_r(double x, int *signgamp) /* wrapper lgamma_r */
#else
	double lgamma_r(x,signgamp)              /* wrapper lgamma_r */
	double x; int *signgamp;
#endif
{
        double y;
        y = __ieee754_lgamma_r(x,signgamp);
#ifndef _IEEE_LIBM
        if(_LIB_VERSION == _IEEE_) return y;
        if(!finite(y)&&finite(x)) {
	      /* lgamma(finite) overflow */
	      errno = ERANGE;
        }
	return y;
#endif
	return y;
}
#endif

#endif /* defined(_DOUBLE_IS_32BITS) */
