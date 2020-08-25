/* w_gammaf.c -- float version of w_gamma.c.
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

#include "math.h"
#include "fdlibm.h"

#if !defined(_IEEE_LIBM) || !defined(HAVE_ALIAS_ATTRIBUTE)
#ifdef __STDC__
	float tgammaf(float x)
#else
	float tgammaf(x)
	float x;
#endif
{
        float y;
	y = __ieee754_tgammaf(x);
#ifdef _IEEE_LIBM
	return y;
#else
	if(_LIB_VERSION == _IEEE_) return y;

	if(!finitef(y)) {
	  if (x==0.0f)
	    /* tgammf pole */
	    return (float)__kernel_standard((double)x,(double)x,142);
	  else if(floorf(x)==x&&x<0.0f)
	    /* tgammaf domain */
	    return (float)__kernel_standard((double)x,(double)x,141);
	  else if (finitef(x))
	    /* tgammaf overflow */
	    return (float)__kernel_standard((double)x,(double)x,140);
	}
	return y;
#endif
}
#endif

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double tgamma(double x)
#else
	double tgamma(x)
	double x;
#endif
{
	return (double) tgammaf((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
