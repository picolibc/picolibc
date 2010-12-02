/* sf_log2.c -- float version of s_log2.c.
 * Modification of sf_exp10.c by Yaakov Selkowitz 2009.
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
 * wrapper log2f(x)
 */

#include "fdlibm.h"
#include <errno.h>
#include <math.h>
#undef log2
#undef log2f

#ifdef __STDC__
	float log2f(float x)		/* wrapper log2f */
#else
	float log2f(x)			/* wrapper log2f */
	float x;
#endif
{
  return (logf(x) / (float_t) M_LN2);
}

#ifdef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double log2(double x)
#else
	double log2(x)
	double x;
#endif
{
	return (double) log2f((float) x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
