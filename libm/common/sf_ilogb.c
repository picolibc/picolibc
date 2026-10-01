/* sf_ilogb.c -- float version of s_ilogb.c.
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

#include <limits.h>
#include "fdlibm.h"

#ifdef LLOGB
#define NAME    llogbf
#define TYPE    long int
#define LOGB0   FP_LLOGB0
#define LOGBNAN FP_LLOGBNAN
#define LOGBINF LONG_MAX
#else
#define NAME    ilogbf
#define TYPE    int
#define LOGB0   FP_ILOGB0
#define LOGBNAN FP_ILOGBNAN
#define LOGBINF INT_MAX
#endif

TYPE
NAME(float x)
{
    __int32_t hx, ix;

    GET_FLOAT_WORD(hx, x);
    hx &= 0x7fffffff;
    if (FLT_UWORD_IS_ZERO(hx)) {
        (void)__math_invalidf(0.0);
        return LOGB0; /* ilogb(0) = special case error */
    }
    if (FLT_UWORD_IS_SUBNORMAL(hx)) {
        for (ix = -126, hx = lsl(hx, 8); hx > 0; hx = lsl(hx, 1))
            ix -= 1;
        return (TYPE)ix;
    }
#if LOGBNAN != LOGBINF
    else if (FLT_UWORD_IS_NAN(hx)) {
        (void)__math_invalidf(0.0);
        return LOGBNAN; /* NAN */
    }
#endif
    else if (!FLT_UWORD_IS_FINITE(hx)) {
        (void)__math_invalidf(0.0);
        return LOGBINF;
    } else
        return (TYPE)(hx >> 23) - 127;
}

#ifdef LLOGB
_MATH_ALIAS_j_f(llogb)
#else
_MATH_ALIAS_i_f(ilogb)
#endif
