
/* @(#)e_remainder.c 5.1 93/09/24 */
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

/* remainder(x,p)
 * Return :
 * 	returns  x REM p  =  x - [x/p]*p as if in infinite
 * 	precise arithmetic, where [x/p] is the (infinite bit)
 *	integer nearest x/p (in half way case choose the even one).
 * Method :
 *	Based on fmod() return x-[x/p]chopped*p exactlp.
 */

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

static const __float64 zero = _F_64(0.0);

__float64
remainder64(__float64 x, __float64 p)
{
    __int32_t hx, hp;
    __uint32_t sx, lx, lp;
    __float64 p_half;

    EXTRACT_WORDS(hx, lx, x);
    EXTRACT_WORDS(hp, lp, p);
    sx = hx & 0x80000000;
    hp &= 0x7fffffff;
    hx &= 0x7fffffff;

    /* purge off exception values */
    if (isnan(x) || isnan(p))
        return x + p;

    if (isinf(x) || (hp | lp) == 0)
        return __math_invalid(x);

    if ((hp | lp) == 0)
        return (x * p) / (x * p); /* p = 0 */

    if (hp <= 0x7fdfffff)
        x = fmod(x, p + p); /* now x < 2p */

    if (((hx - hp) | (lx - lp)) == 0)
        return zero * x;
    x = fabs64(x);
    p = fabs64(p);
    if (hp < 0x00200000) {
        if (x + x > p) {
            x -= p;
            if (x + x >= p)
                x -= p;
        }
    } else {
        p_half = _F_64(0.5) * p;
        if (x > p_half) {
            x -= p;
            if (x >= p_half)
                x -= p;
        }
    }
    GET_HIGH_WORD(hx, x);
    SET_HIGH_WORD(x, hx ^ sx);
    return x;
}

_MATH_ALIAS_d_dd(remainder)

#endif /* _NEED_FLOAT64 */
