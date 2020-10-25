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

#if !defined(_IEEE_LIBM) || !defined(HAVE_ALIAS_ATTRIBUTE)
#ifdef __STDC__
	float lgammaf_r(float x, int *signgamp) /* wrapper lgammaf_r */
#else
	float lgammaf_r(x,signgamp)              /* wrapper lgammaf_r */
	float x; int *signgamp;
#endif
{
        float y;
        y = __ieee754_lgammaf_r(x,signgamp);
#ifndef _IEEE_LIBM
        if(_LIB_VERSION == _IEEE_) return y;
        if(!finitef(y)&&finitef(x)) {
	    /* lgammaf(finite) overflow */
	    errno = ERANGE;
	}
#endif
	return y;
}
#endif
