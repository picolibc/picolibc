
/* @(#)e_atanh.c 5.1 93/09/24 */
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

/* atanh(x)
 * Method :
 *    1.Reduced x to positive by atanh(-x) = -atanh(x)
 *    2.For x>=0.5
 *                  1              2x                          x
 *	atanh(x) = --- * log(1 + -------) = 0.5 * log1p(2 * --------)
 *                  2             1 - x                      1 - x
 *
 * 	For x<0.5
 *	atanh(x) = 0.5*log1p(2x+2x*x/(1-x))
 *
 * Special cases:
 *	atanh(x) is NaN if |x| > 1 with signal;
 *	atanh(NaN) is that NaN with no signal;
 *	atanh(+-1) is +-INF with signal.
 *
 */

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

static const __float64 one = _F_64(1.0), huge = _F_64(1e300);

static const __float64 zero = _F_64(0.0);

__float64
atanh64(__float64 x)
{
    __float64 t;
    __int32_t hx, ix;
    __uint32_t lx;
    EXTRACT_WORDS(hx, lx, x);
    ix = hx & 0x7fffffff;
    if ((ix | ((lx | (-lx)) >> 31)) > 0x3ff00000) /* |x|>1 */
        return __math_invalid(x);
    if (ix == 0x3ff00000)
        return __math_divzero(x < 0);
    if (ix < 0x3e300000 && (huge + x) > zero)
        return x; /* x<2**-28 */
    SET_HIGH_WORD(x, ix);
    if (ix < 0x3fe00000) { /* x < 0.5 */
        t = x + x;
        t = _F_64(0.5) * log1p64(t + t * x / (one - x));
    } else
        t = _F_64(0.5) * log1p64((x + x) / (one - x));
    if (hx >= 0)
        return t;
    else
        return -t;
}

_MATH_ALIAS_d_d(atanh)

#endif /* _NEED_FLOAT64 */
