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

#define _GNU_SOURCE
#include "fdlibm.h"

static const __float64 two52 = _F_64(0x1p52), one = 1, zero = 0, pi = _F_64(3.14159265358979323851);

__float64
sinpi64(__float64 x)
{
    __float64 y, z;
    __int32_t n, ix;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;

    if (ix < 0x3fd00000)
        return __kernel_sin(pi * x, zero, 0);
    y = -x; /* x is assume negative */

    /*
     * argument reduction, make sure inexact flag not raised if input
     * is an integer
     */
    z = floor64(y);
    if (z != y) { /* inexact anyway */
        y *= _F_64(0.5);
        y = _F_64(2.0) * (y - floor64(y)); /* y = |x| mod 2.0 */
        n = (__int32_t)(y * _F_64(4.0));
    } else {
        if (ix >= 0x43400000) {
            y = zero;
            n = 0; /* y must be even */
        } else {
            if (ix < 0x43300000)
                z = y + two52; /* exact */
            GET_LOW_WORD(n, z);
            n &= 1;
            y = n;
            n <<= 2;
        }
    }
    switch (n) {
    case 0:
        y = __kernel_sin(pi * y, zero, 0);
        break;
    case 1:
    case 2:
        y = __kernel_cos(pi * (_F_64(0.5) - y), zero);
        break;
    case 3:
    case 4:
        y = __kernel_sin(pi * (one - y), zero, 0);
        break;
    case 5:
    case 6:
        y = -__kernel_cos(pi * (y - _F_64(1.5)), zero);
        break;
    default:
        y = __kernel_sin(pi * (y - _F_64(2.0)), zero, 0);
        break;
    }
    return -y;
}
