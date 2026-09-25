
/* @(#)s_nextafter.c 5.1 93/09/24 */
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
FUNCTION
       <<nextafter>>, <<nextafterf>>---get next number

INDEX
        nextafter
INDEX
        nextafterf

SYNOPSIS
       #include <math.h>
       double nextafter(double <[val]>, double <[dir]>);
       float nextafterf(float <[val]>, float <[dir]>);

DESCRIPTION
<<nextafter>> returns the double-precision floating-point number
closest to <[val]> in the direction toward <[dir]>.  <<nextafterf>>
performs the same operation in single precision.  For example,
<<nextafter(0.0,1.0)>> returns the smallest positive number which is
representable in double precision.

RETURNS
Returns the next closest number to <[val]> in the direction toward
<[dir]>.

PORTABILITY
        Neither <<nextafter>> nor <<nextafterf>> is required by ANSI C
        or by the System V Interface Definition (Issue 2).
*/

/* IEEE functions
 *	nextafter(x,y)
 *	return the next machine floating-point number of x in the
 *	direction toward y.
 *   Special cases:
 */

#define _ISOC23_SOURCE
#include "fdlibm.h"

#ifdef _NEED_FLOAT64

__float64
#if defined(NEXTUP)
nextup64(__float64 x)
#elif defined(NEXTDOWN)
nextdown64(__float64 x)
#else
nextafter64(__float64 x, __float64 y)
#endif

{
    __int32_t  hx, ix;
    __uint32_t lx;

    EXTRACT_WORDS(hx, lx, x);
    ix = hx & 0x7fffffff; /* |x| */

#if defined(NEXTUP)
#define hy ((__int32_t)0x7ff00000)
#define ly ((__uint32_t)0)
#define iy ((__int32_t)0x7ff00000)
#define y x
#elif defined(NEXTDOWN)
#define hy ((__int32_t)0xfff00000)
#define ly ((__uint32_t)0)
#define iy ((__int32_t)0x7ff00000)
#define y x
#else
    __int32_t  hy, iy;
    __uint32_t ly;

    EXTRACT_WORDS(hy, ly, y);
    iy = hy & 0x7fffffff; /* |y| */

#define CHECK_EQUAL
#endif

    if (((ix >= 0x7ff00000) && ((ix - 0x7ff00000) | lx) != 0) || /* x is nan */
        ((iy >= 0x7ff00000) && ((iy - 0x7ff00000) | ly) != 0))   /* y is nan */
        return x + y;

#ifdef CHECK_EQUAL
    if (x == y)
        return y; /* x=y, return y */
#endif

    if ((ix | lx) == 0) {                    /* x == 0 */
        INSERT_WORDS(x, hy & 0x80000000, 1); /* return +-minsubnormal */
        force_eval_float64(opt_barrier_float64(x) * x);
        return x;
    }
    if (hx >= 0) {                                  /* x > 0 */
        if (hx > hy || ((hx == hy) && (lx > ly))) { /* x > y, x -= ulp */
            if (lx == 0)
                hx -= 1;
            lx -= 1;
        } else { /* x < y, x += ulp */
            lx += 1;
            if (lx == 0)
                hx += 1;
        }
    } else {                                                   /* x < 0 */
        if (hy >= 0 || hx > hy || ((hx == hy) && (lx > ly))) { /* x < y, x -= ulp */
            if (lx == 0)
                hx -= 1;
            lx -= 1;
        } else { /* x > y, x += ulp */
            lx += 1;
            if (lx == 0)
                hx += 1;
        }
    }
    ix = hx & 0x7ff00000;
    if (ix >= 0x7ff00000)
        return __math_oflow(hx < 0); /* overflow  */
    INSERT_WORDS(x, hx, lx);
    if (ix < 0x00100000) /* underflow */
        return __math_denorm(x);
    return (x);
}

#if defined(NEXTUP)
_MATH_ALIAS_d_dd(nextup)
#elif defined(NEXTDOWN)
_MATH_ALIAS_d_dd(nextdown)
#else
_MATH_ALIAS_d_dd(nextafter)
#endif

#endif /* _NEED_FLOAT64 */
