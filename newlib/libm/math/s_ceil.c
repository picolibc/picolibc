
/* @(#)s_ceil.c 5.1 93/09/24 */
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
 * ceil(x)
 * Return x rounded toward -inf to integral value
 * Method:
 *	Bit twiddling.
 * Exception:
 *	Inexact flag raised if x not equal to ceil(x).
 */

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

__float64
ceil64(__float64 x)
{
    __int32_t i0, i1, j0, j;
    __uint32_t i;
    EXTRACT_WORDS(i0, i1, x);
    j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
    if (j0 < 20) {
        if (j0 < 0) {
            if (i0 < 0) {
                i0 = 0x80000000;
                i1 = 0;
            } else if ((i0 | i1) != 0) {
                i0 = 0x3ff00000;
                i1 = 0;
            }
        } else {
            i = (0x000fffff) >> j0;
            if (((i0 & i) | i1) == 0)
                return x; /* x is integral */
            if (i0 > 0)
                i0 += (0x00100000) >> j0;
            i0 &= (~i);
            i1 = 0;
        }
    } else if (j0 > 51) {
        if (j0 == 0x400)
            return x + x; /* inf or NaN */
        else
            return x; /* x is integral */
    } else {
        i = ((__uint32_t)(0xffffffffUL)) >> (j0 - 20);
        if ((i1 & i) == 0)
            return x; /* x is integral */
        if (i0 > 0) {
            if (j0 == 20)
                i0 += 1;
            else {
                j = i1 + ((__uint32_t) 1 << (52 - j0));
                if ((__uint32_t) j < (__uint32_t) i1)
                    i0 += 1; /* got a carry */
                i1 = j;
            }
        }
        i1 &= (~i);
    }
    INSERT_WORDS(x, i0, i1);
    return x;
}

_MATH_ALIAS_d_d(ceil)

#endif /* _NEED_FLOAT64 */
