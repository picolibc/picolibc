/* wf_exp2.c -- float version of w_exp2.c.
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
 * wrapper exp2f(x)
 */

#include "fdlibm.h"
#if __OBSOLETE_MATH_FLOAT
#include <errno.h>
#include <math.h>

float __inhibit_new_builtin_calls
exp2f(float x) /* wrapper exp2f */
{
    return _powf(2.0, x);
}

_MATH_ALIAS_f_f(exp2)

#else
#include "../common/sf_exp2.c"
#endif /* __OBSOLETE_MATH_FLOAT */
