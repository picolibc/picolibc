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

#define _GNU_SOURCE
#include <math.h>
#include "fdlibm.h"

float
nearbyintf(float x)
{
    if (isnan(x))
        return x + x;
#if defined(FE_INEXACT)
    fenv_t env;
    fegetenv(&env);
    fedisableexcept(FE_INEXACT);
#endif
    x = rintf(x);
#if defined(FE_INEXACT)
    fesetenv(&env);
#endif
    return x;
}

_MATH_ALIAS_f_f(nearbyint)
