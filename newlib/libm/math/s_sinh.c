
/* @(#)e_sinh.c 5.1 93/09/24 */
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

/* sinh(x)
 * Method :
 * mathematically sinh(x) if defined to be (exp(x)-exp(-x))/2
 *	1. Replace x by |x| (sinh(-x) = -sinh(x)).
 *	2.
 *		                                    E + E/(E+1)
 *	    0        <= x <= 22     :  sinh(x) := --------------, E=expm1(x)
 *			       			        2
 *
 *	    22       <= x <= lnovft :  sinh(x) := exp(x)/2
 *	    lnovft   <= x <= ln2ovft:  sinh(x) := exp(x/2)/2 * exp(x/2)
 *	    ln2ovft  <  x	    :  sinh(x) := x*shuge (overflow)
 *
 * Special cases:
 *	sinh(x) is |x| if x is +INF, -INF, or NaN.
 *	only sinh(0)=0 is exact for finite x.
 */

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

static const __float64 one = _F_64(1.0);

__float64
sinh64(__float64 x)
{
    __float64 t, w, h;
    __int32_t ix, jx;
    __uint32_t lx;

    /* High word of |x|. */
    GET_HIGH_WORD(jx, x);
    ix = jx & 0x7fffffff;

    /* x is INF or NaN */
    if (ix >= 0x7ff00000)
        return x + x;

    h = _F_64(0.5);
    if (jx < 0)
        h = -h;
    /* |x| in [0,22], return sign(x)*0.5*(E+E/(E+1))) */
    if (ix < 0x40360000) { /* |x|<22 */
        if (ix < 0x3e300000) /* |x|<2**-28 */
            return __math_inexact64(x); /* sinh(tiny) = tiny with inexact */
        t = expm164(fabs64(x));
        if (ix < 0x3ff00000)
            return h * (_F_64(2.0) * t - t * t / (t + one));
        return h * (t + t / (t + one));
    }

    /* |x| in [22, log(maxdouble)] return 0.5*exp(|x|) */
    if (ix < 0x40862E42)
        return h * exp64(fabs64(x));

    /* |x| in [log(maxdouble), overflowthresold] */
    GET_LOW_WORD(lx, x);
    if (ix < 0x408633CE || (ix == 0x408633ce && lx <= (__uint32_t)0x8fb9f87d)) {
        w = exp64(_F_64(0.5) * fabs64(x));
        t = h * w;
        return t * w;
    }

    /* |x| > overflowthresold, sinh(x) overflow */
    return __math_oflow(x < 0);
}

_MATH_ALIAS_d_d(sinh)

#endif /* _NEED_FLOAT64 */
