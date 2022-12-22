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


#include <math.h>
#include "fdlibm.h"

float nearbyintf(float x)
{
    if (isnan(x)) return x + x;
#if defined(FE_INEXACT) && !defined(PICOLIBC_DOUBLE_NOEXECPT)
    fenv_t env;
    fegetenv(&env);
#endif
    x = rintf(x);
#if defined(FE_INEXACT) && !defined(PICOLIBC_DOUBLE_NOEXECPT)
    fesetenv(&env);
#endif
    return x;
}

_MATH_ALIAS_f_f(nearbyint)
