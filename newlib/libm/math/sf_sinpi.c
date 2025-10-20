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

#define _ISOC23_SOURCE
#include "fdlibm.h"

static const float two23 = 0x1p23, one = 1, zero = 0, pi = 3.141592653589793;

float
sinpif(float x)
{
    float y, z;
    __int32_t n, ix;

    GET_FLOAT_WORD(ix, x);
    ix &= 0x7fffffff;

    if (ix < 0x3e800000)
        return __kernel_sinf(pi * x, zero, 0);
    y = -x; /* x is assume negative */

    /*
     * argument reduction, make sure inexact flag not raised if input
     * is an integer
     */
    z = floorf(y);
    if (z != y) { /* inexact anyway */
        y *= (float)0.5;
        y = (float)2.0 * (y - floorf(y)); /* y = |x| mod 2.0 */
        n = (__int32_t)(y * (float)4.0);
    } else {
        if (ix >= 0x4b800000) {
            y = zero;
            n = 0; /* y must be even */
        } else {
            if (ix < 0x4b000000)
                z = y + two23; /* exact */
            GET_FLOAT_WORD(n, z);
            n &= 1;
            y = n;
            n <<= 2;
        }
    }
    switch (n) {
    case 0:
        y = __kernel_sinf(pi * y, zero, 0);
        break;
    case 1:
    case 2:
        y = __kernel_cosf(pi * ((float)0.5 - y), zero);
        break;
    case 3:
    case 4:
        y = __kernel_sinf(pi * (one - y), zero, 0);
        break;
    case 5:
    case 6:
        y = -__kernel_cosf(pi * (y - (float)1.5), zero);
        break;
    default:
        y = __kernel_sinf(pi * (y - (float)2.0), zero, 0);
        break;
    }
    return -y;
}
