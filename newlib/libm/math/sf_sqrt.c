/* ef_sqrtf.c -- float version of e_sqrt.c.
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

float
sqrtf(float x)
{
    float z;
    __uint32_t r, hx;
    __int32_t ix, s, q, m, t, i;

    GET_FLOAT_WORD(ix, x);
    hx = ix & 0x7fffffff;

    /* take care of Inf and NaN */
    if (!FLT_UWORD_IS_FINITE(hx)) {
        if (ix < 0 && !isnanf(x))
            return __math_invalidf(x); /* sqrt(-inf)=sNaN */
        return x + x; /* sqrt(NaN)=NaN, sqrt(+inf)=+inf */
    }

    /* take care of zero and -ves */
    if (FLT_UWORD_IS_ZERO(hx))
        return x; /* sqrt(+-0) = +-0 */
    if (ix < 0)
        return __math_invalidf(x); /* sqrt(-ve) = sNaN */

    /* normalize x */
    m = (ix >> 23);
    if (FLT_UWORD_IS_SUBNORMAL(hx)) { /* subnormal x */
        for (i = 0; (ix & 0x00800000L) == 0; i++)
            ix <<= 1;
        m -= i - 1;
    }
    m -= 127; /* unbias exponent */
    ix = (ix & 0x007fffffL) | 0x00800000L;
    if (m & 1) /* odd m, double x to make it even */
        ix += ix;
    m >>= 1; /* m = [m/2] */

    /* generate sqrt(x) bit by bit */
    ix += ix;
    q = s = 0; /* q = sqrt(x) */
    r = 0x01000000L; /* r = moving bit from right to left */

    while (r != 0) {
        t = s + r;
        if (t <= ix) {
            s = t + r;
            ix -= t;
            q += r;
        }
        ix += ix;
        r >>= 1;
    }

    if (ix != 0) {
        FE_DECL_ROUND(rnd);
        if (__is_nearest(rnd))
            q += (q & 1);
        else if (__is_upward(rnd))
            q += 2;
    }
    ix = (q >> 1) + 0x3f000000L;
    ix += (m << 23);
    SET_FLOAT_WORD(z, ix);
    return z;
}

_MATH_ALIAS_f_f(sqrt)
