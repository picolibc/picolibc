/* ef_atanh.c -- float version of e_atanh.c.
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

static const float one = 1.0;

float
atanhf(float x)
{
    float t;
    __int32_t hx, ix;
    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7fffffff;
    if (ix > 0x3f800000) /* |x|>1 */
        return __math_invalidf(x);
    if (ix == 0x3f800000)
        return __math_divzerof(x < 0);
    if (ix < 0x31800000) /* x<2**-28 */
        return __math_inexactf(x);
    SET_FLOAT_WORD(x, ix);
    if (ix < 0x3f000000) { /* x < 0.5 */
        t = x + x;
        t = (float)0.5 * log1pf(t + t * x / (one - x));
    } else
        t = (float)0.5 * log1pf((x + x) / (one - x));
    if (hx >= 0)
        return t;
    else
        return -t;
}

_MATH_ALIAS_f_f(atanh)
