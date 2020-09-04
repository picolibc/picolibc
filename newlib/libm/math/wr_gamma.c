
/* @(#)wr_gamma.c 5.1 93/09/24 */
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
 * wrapper double gamma_r(double x, int *signgamp)
 */

#include "fdlibm.h"
#include <errno.h>

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double gamma_r(double x, int *signgamp) /* wrapper lgamma_r */
#else
	double gamma_r(x,signgamp)              /* wrapper lgamma_r */
	double x; int *signgamp;
#endif
{
	return lgamma_r(x, signgamp);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
