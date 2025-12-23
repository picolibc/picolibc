
/* @(#)s_finite.c 5.1 93/09/24 */
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
 * finite(x) returns 1 is x is finite, else 0;
 * no branching!
 */

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

int
finite64(__float64 x)
{
    __int32_t hx;
    GET_HIGH_WORD(hx, x);
    return (int)((__uint32_t)((hx & 0x7fffffff) - 0x7ff00000) >> 31);
}

#ifdef __strong_reference
__strong_reference(finite64, __finite64);
#else
int
__finite64(__float64 x)
{
    return finite64(x);
}
#endif

_MATH_ALIAS_i_d(finite) _MATH_ALIAS_i_d(__finite)

#endif /* _NEED_FLOAT64 */
