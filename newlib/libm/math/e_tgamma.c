
/* @(#)e_tgamma.c 5.1 93/09/24 */
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

/* __ieee754_tgamma(x)
 * Gamma function. Returns gamma(x)
 *
 * Method: See __ieee754_lgamma_r
 */

#include "fdlibm.h"

#ifdef __STDC__
	double __ieee754_tgamma(double x)
#else
	double __ieee754_tgamma(x)
	double x;
#endif
{
	int signgam_local;
	double y = __ieee754_exp(__ieee754_lgamma_r(x, &signgam_local));
	if (signgam_local < 0)
		y = -y;
	return y;
}
