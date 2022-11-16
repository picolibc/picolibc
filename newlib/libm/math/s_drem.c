/*
Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.

Developed at SunPro, a Sun Microsystems, Inc. business.
Permission to use, copy, modify, and distribute this
software is freely granted, provided that this notice
is preserved.
 */
/*
 * drem() wrapper for remainder().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

__float64
drem64(__float64 x, __float64 y)
{
    return remainder64(x, y);
}

_MATH_ALIAS_d_dd(drem)

#endif
