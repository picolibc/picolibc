
/* @(#)e_scalb.c 5.1 93/09/24 */
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
 * scalb(x, fn) is provide for
 * passing various standard test suite. One
 * should use scalbn() instead.
 */

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

__float64
scalb64(__float64 x, __float64 fn)
{
    if (isnan(fn) || isnan(x))
        return x + fn;

    if (isinf(fn)) {
        if ((x == 0.0 && fn > 0.0) || (isinf(x) && fn < 0.0))
            return __math_invalid(fn);
        if (fn > 0.0)
            return fn*x;
        else
            return x/(-fn);
    }

    if (rint64(fn) != fn)
        return __math_invalid(fn);

    if (fn > 4 * __DBL_MAX_EXP__)
        fn = 4 * __DBL_MAX_EXP__;

    if (fn < -4 * __DBL_MAX_EXP__)
        fn = -4 * __DBL_MAX_EXP__;

    return scalbn64(x, (int)fn);
}

_MATH_ALIAS_d_dd(scalb)

#endif /* _NEED_FLOAT64 */
