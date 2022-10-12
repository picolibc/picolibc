/* wf_log2.c -- float version of s_log2.c.
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
#if __OBSOLETE_MATH_FLOAT
#include <errno.h>
#include <math.h>
#undef log2
#undef log2f

static const float log2_inv = 1.0f / (float) M_LN2;

float
log2f(float x) /* wrapper log2f */
{
    return logf(x) * log2_inv;
}

#ifdef _DOUBLE_IS_32BITS

double
log2(double x)
{
    return (double)log2f((float)x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
#endif /* __OBSOLETE_MATH_FLOAT */
