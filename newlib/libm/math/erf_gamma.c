/* erf_gamma.c -- float version of er_gamma.c.
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

/* __ieee754_gammaf_r(x, signgamp)
 * Reentrant version of the logarithm of the Gamma function 
 * with user provide pointer for the sign of Gamma(x). 
 *
 * Method: See __ieee754_lgammaf
 */

#include "fdlibm.h"

#if defined(_IEEE_LIBM) && defined(HAVE_ALIAS_ATTRIBUTE)
__strong_reference(__ieee754_gammaf, gammaf);
#endif

#ifdef __STDC__
	float __ieee754_gammaf(float x)
#else
	float __ieee754_gammaf(x)
	float x;
#endif
{
	return __ieee754_expf (__ieee754_lgammaf(x));
}
