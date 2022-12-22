/* ef_remainder.c -- float version of e_remainder.c.
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

static const float zero = 0.0;

float
remainderf(float x, float p)
{
    __int32_t hx, hp;
    __uint32_t sx;
    float p_half;

    GET_FLOAT_WORD(hx, x);
    GET_FLOAT_WORD(hp, p);
    sx = hx & 0x80000000;
    hp &= 0x7fffffff;
    hx &= 0x7fffffff;

    /* purge off exception values */
    if (FLT_UWORD_IS_NAN(hx) || FLT_UWORD_IS_NAN(hp))
        return x + p;

    if (isinf(x) || hp == 0)
        return __math_invalidf(x);

    if (hp <= FLT_UWORD_HALF_MAX)
        x = fmodf(x, p + p); /* now x < 2p */
    if ((hx - hp) == 0)
        return zero * x;
    x = fabsf(x);
    p = fabsf(p);
    if (hp < 0x01000000) {
        if (x + x > p) {
            x -= p;
            if (x + x >= p)
                x -= p;
        }
    } else {
        p_half = (float)0.5 * p;
        if (x > p_half) {
            x -= p;
            if (x >= p_half)
                x -= p;
        }
    }
    GET_FLOAT_WORD(hx, x);
    SET_FLOAT_WORD(x, hx ^ sx);
    return x;
}

_MATH_ALIAS_f_ff(remainder)
