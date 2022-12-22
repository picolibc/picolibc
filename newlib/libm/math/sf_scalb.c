/* ef_scalb.c -- float version of e_scalb.c.
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
#include <limits.h>

float
scalbf(float x, float fn)
{
    if (isnan(fn) || isnan(x))
        return x + fn;

    if (isinf(fn)) {
        if ((x == 0.0f && fn > 0.0f) || (isinf(x) && fn < 0.0f))
            return __math_invalidf(fn);
        if (fn > 0.0f)
            return fn*x;
        else
            return x/(-fn);
    }

    if (rintf(fn) != fn)
        return __math_invalidf(fn);

    if (fn > 4 * __FLT_MAX_EXP__)
        fn = 4 * __FLT_MAX_EXP__;

    if (fn < -4 * __FLT_MAX_EXP__)
        fn = -4 * __FLT_MAX_EXP__;

    return scalbnf(x, (int)fn);
}

_MATH_ALIAS_f_ff(scalb)
