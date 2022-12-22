/* sf_tan.c -- float version of s_tan.c.
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
tanf(float x)
{
    float y[2], z = 0.0;
    __int32_t n, ix;

    GET_FLOAT_WORD(ix, x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if (ix <= 0x3f490fda)
        return __kernel_tanf(x, z, 1);

    /* tan(Inf or NaN) is NaN */
    else if (!FLT_UWORD_IS_FINITE(ix))
        return __math_invalidf(x); /* NaN */

    /* argument reduction needed */
    else {
        n = __rem_pio2f(x, y);
        return __kernel_tanf(y[0], y[1], 1 - ((n & 1) << 1)); /*   1 -- n even
							      -1 -- n odd */
    }
}

_MATH_ALIAS_f_f(tan)
