/* wf_lgamma.c -- float version of w_lgamma.c.
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

#include "fdlibm.h"
#include <errno.h>

float
lgammaf(float x)
{
    return lgammaf_r(x, &__signgam);
}

#ifdef _DOUBLE_IS_32BITS

double
lgamma(double x)
{
    return (double)lgammaf((float)x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
