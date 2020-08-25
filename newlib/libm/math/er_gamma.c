
/* @(#)er_gamma.c 5.1 93/09/24 */
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

/* __ieee754_gamma_r(x, signgamp)
 * Reentrant version of the logarithm of the Gamma function 
 * with user provide pointer for the sign of Gamma(x). 
 *
 * Method: See __ieee754_lgamma_r
 */

#include "fdlibm.h"

#if defined(_IEEE_LIBM) && defined(HAVE_ALIAS_ATTRIBUTE)
__strong_reference(__ieee754_gamma, gamma);
__strong_reference(__ieee754_gamma, tgamma);
#endif

#ifdef __STDC__
	double __ieee754_gamma(double x)
#else
	double __ieee754_gamma(x)
	double x;
#endif
{
	int sign;
	double y;
	sign = 1;
	y = __ieee754_exp(___ieee754_lgamma_r(x, &sign));
	if (sign < 0)
		y = -y;
	return y;
}
