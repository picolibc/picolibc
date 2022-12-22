/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "fdlibm.h"

float fmaxf(float x, float y)
{
    if (issignaling(x) || issignaling(y))
        return x + y;

    if (isnan(x))
        return y;

    if (isnan(y))
        return x;

    return x > y ? x : y;
}

_MATH_ALIAS_f_ff(fmax)
