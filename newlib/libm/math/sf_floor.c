/* sf_floor.c -- float version of s_floor.c.
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
 * floorf(x)
 * Return x rounded toward -inf to integral value
 * Method:
 *	Bit twiddling.
 * Exception:
 *	Inexact flag raised if x not equal to floorf(x).
 */

#include "fdlibm.h"

float
floorf(float x)
{
    __int32_t i0, j0;
    __uint32_t i, ix;
    GET_FLOAT_WORD(i0, x);
    ix = (i0 & 0x7fffffff);
    j0 = (ix >> 23) - 0x7f;
    if (j0 < 23) {
        if (j0 < 0) { /* raise inexact if x != 0 */
            if (i0 >= 0) {
                i0 = 0;
            } else if (!FLT_UWORD_IS_ZERO(ix)) {
                i0 = 0xbf800000;
            }
        } else {
            i = (0x007fffff) >> j0;
            if ((i0 & i) == 0)
                return x; /* x is integral */
            if (i0 < 0)
                i0 += (0x00800000) >> j0;
            i0 &= (~i);
        }
    } else {
        if (!FLT_UWORD_IS_FINITE(ix))
            return x + x; /* inf or NaN */
        else
            return x; /* x is integral */
    }
    SET_FLOAT_WORD(x, i0);
    return x;
}

_MATH_ALIAS_f_f(floor)
