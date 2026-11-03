/* sf_nextafter.c -- float version of s_nextafter.c.
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

float
#if defined(NEXTUP)
nextupf(float x)
#elif defined(NEXTDOWN)
nextdownf(float x)
#else
nextafterf(float x, float y)
#endif
{
    __int32_t hx, ix;

    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7fffffff; /* |x| */
#if defined(NEXTUP)
#define hy ((__int32_t)0x7f800000)
#define iy ((__int32_t)0x7f800000)
#define y  x
#elif defined(NEXTDOWN)
#define hy ((__int32_t)0xff800000)
#define iy ((__int32_t)0x7f800000)
#define y  x
#else
    __int32_t hy, iy;
    GET_FLOAT_WORD(hy, y);
    iy = hy & 0x7fffffff; /* |y| */
#define CHECK_EQUAL
#define EXCEPT
#endif

    if (FLT_UWORD_IS_NAN(ix) || FLT_UWORD_IS_NAN(iy))
        return clang_barrier_float(x) + y;
#ifdef CHECK_EQUAL
    if (x == y)
        return y; /* x=y, return y */
#else
    /* Avoid FE_OVERFLOW when x is infinity */
    if (hx == hy)
        return x;
#endif
    if (FLT_UWORD_IS_ZERO(ix)) { /* x == 0 */
        SET_FLOAT_WORD(x, (hy & 0x80000000) | FLT_UWORD_MIN);
#ifdef EXCEPT
        force_eval_float(opt_barrier_float(x) * x);
#endif
        return x;
    }
    if (hx >= 0) {     /* x > 0 */
        if (hx > hy) { /* x > y, x -= ulp */
            hx -= 1;
        } else { /* x < y, x += ulp */
            hx += 1;
        }
    } else {                      /* x < 0 */
        if (hy >= 0 || hx > hy) { /* x < y, x -= ulp */
            hx -= 1;
        } else { /* x > y, x += ulp */
            hx += 1;
        }
    }
#ifdef EXCEPT
    ix = hx & 0x7f800000;
    if (ix > FLT_UWORD_MAX)
        return check_oflowf(x + x); /* overflow  */
#endif
    SET_FLOAT_WORD(x, hx);
#ifdef EXCEPT
    if (ix < 0x00800000) /* underflow */
        return __math_denormf(x);
#endif
    return x;
}

#undef y

#if defined(NEXTUP)
_MATH_ALIAS_f_f(nextup)
#elif defined(NEXTDOWN)
_MATH_ALIAS_f_f(nextdown)
#else
_MATH_ALIAS_f_ff(nextafter)
#endif
