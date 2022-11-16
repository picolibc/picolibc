/* ef_tgamma.c -- float version of e_tgamma.c.
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
 *
 */

/* tgammaf(x)
 * Float version the Gamma function. Returns gamma(x)
 *
 * Method: See lgammaf
 */

#include "fdlibm.h"

float
tgammaf(float x)
{
    int signgam_local;
    int divzero = 0;

    if (isless(x, 0.0f) && clang_barrier_float(rintf(x)) == x)
        return __math_invalidf(x);

    float y = expf(__math_lgammaf_r(x, &signgam_local, &divzero));
    if (signgam_local < 0)
        y = -y;
    if (isinff(y) && finitef(x) && !divzero)
        return __math_oflowf(signgam_local < 0);
    return y;
}

_MATH_ALIAS_f_f(tgamma)
