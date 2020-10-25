
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
#include <errno.h>

#ifndef _DOUBLE_IS_32BITS

#if !defined(_IEEE_LIBM) || !defined(HAVE_ALIAS_ATTRIBUTE)
#ifdef __STDC__
	double lgamma(double x)
#else
	double lgamma(x)
	double x;
#endif
{
	return lgamma_r(x, &signgam);
}
#endif

#endif /* defined(_DOUBLE_IS_32BITS) */







