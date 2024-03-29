/* sf_tanh.c -- float version of s_tanh.c.
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

static const float one = 1.0, two = 2.0;

float
tanhf(float x)
{
    float t, z;
    __int32_t jx, ix;

    GET_FLOAT_WORD(jx, x);
    ix = jx & 0x7fffffff;

    /* x is INF or NaN */
    if (!FLT_UWORD_IS_FINITE(ix)) {
        if (jx >= 0)
            return one / x + one; /* tanh(+-inf)=+-1 */
        else
            return one / x - one; /* tanh(NaN) = NaN */
    }

    /* |x| < 22 */
    if (ix < 0x41b00000) { /* |x|<22 */
        if (ix < 0x24000000) /* |x|<2**-55 */
            return x * (one + x); /* tanh(small) = small */
        if (ix >= 0x3f800000) { /* |x|>=1  */
            t = expm1f(two * fabsf(x));
            z = one - two / (t + two);
        } else {
            t = expm1f(-two * fabsf(x));
            z = -t / (t + two);
        }
        /* |x| > 22, return +-1 */
    } else {
        z = __math_inexactf(one);
    }
    return (jx >= 0) ? z : -z;
}

_MATH_ALIAS_f_f(tanh)
