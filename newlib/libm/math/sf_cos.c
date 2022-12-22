/* sf_cos.c -- float version of s_cos.c.
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

#include "fdlibm.h"
#if __OBSOLETE_MATH_FLOAT

float
cosf(float x)
{
    float y[2], z = 0.0;
    __int32_t n, ix;

    GET_FLOAT_WORD(ix, x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if (ix <= 0x3f490fd8)
        return __kernel_cosf(x, z);

    /* cos(Inf or NaN) is NaN */
    else if (!FLT_UWORD_IS_FINITE(ix))
        return __math_invalidf(x);

    /* argument reduction needed */
    else {
        n = __rem_pio2f(x, y);
        switch (n & 3) {
        case 0:
            return __kernel_cosf(y[0], y[1]);
        case 1:
            return -__kernel_sinf(y[0], y[1], 1);
        case 2:
            return -__kernel_cosf(y[0], y[1]);
        default:
            return __kernel_sinf(y[0], y[1], 1);
        }
    }
}

#if defined(_HAVE_ALIAS_ATTRIBUTE)
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(cosf, _cosf);
#endif

_MATH_ALIAS_f_f(cos)
#else
#include "../common/cosf.c"
#endif /* __OBSOLETE_MATH_FLOAT */
