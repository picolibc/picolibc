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

/* IEEE functions
 *	nextafterl(x,y)
 *	return the next machine floating-point number of x in the
 *	direction toward y.
 *   Special cases:
 */

long double
#if defined(NEXTUP)
nextupl(long double x)
#elif defined(NEXTDOWN)
nextdownl(long double x)
#else
nextafterl(long double x, long double y)
#endif
{
    int64_t   hx, ix;
    u_int64_t lx;

    GET_LDOUBLE_WORDS64(hx, lx, x);
    ix = hx & 0x7fffffffffffffffLL; /* |x| */

#if defined(NEXTUP)
#define hy ((int64_t)0x7fff000000000000LL)
#define iy ((int64_t)0x7fff000000000000LL)
#define ly ((u_int64_t)0)
#define y  ((long double)INFINITY)
#elif defined(NEXTDOWN)
#define hy ((int64_t)0xffff000000000000LL)
#define iy ((int64_t)0x7fff000000000000LL)
#define ly ((u_int64_t)0)
#define y  ((long double)-INFINITY)
#else
    int64_t   hy, iy;
    u_int64_t ly;
    GET_LDOUBLE_WORDS64(hy, ly, y);
    iy = hy & 0x7fffffffffffffffLL; /* |y| */
#endif

    if (((ix >= 0x7fff000000000000LL) && ((ix - 0x7fff000000000000LL) | lx) != 0) || /* x is nan */
        ((iy >= 0x7fff000000000000LL) && ((iy - 0x7fff000000000000LL) | ly) != 0))   /* y is nan */
        return x + y;
    if (x == y)
        return y;                                              /* x=y, return y */
    if ((ix | lx) == 0) {                                      /* x == 0 */
        SET_LDOUBLE_WORDS64(x, hy & 0x8000000000000000ULL, 1); /* return +-minsubnormal */
        force_eval_long_double(opt_barrier_long_double(x) * x);
        return x;
    }
    if (hx >= 0) {                                  /* x > 0 */
        if (hx > hy || ((hx == hy) && (lx > ly))) { /* x > y, x -= ulp */
            if (lx == 0)
                hx--;
            lx--;
        } else { /* x < y, x += ulp */
            lx++;
            if (lx == 0)
                hx++;
        }
    } else {                                                   /* x < 0 */
        if (hy >= 0 || hx > hy || ((hx == hy) && (lx > ly))) { /* x < y, x -= ulp */
            if (lx == 0)
                hx--;
            lx--;
        } else { /* x > y, x += ulp */
            lx++;
            if (lx == 0)
                hx++;
        }
    }
    ix = hx & 0x7fff000000000000LL;
    if (ix == 0x7fff000000000000LL)
        return __math_oflowl(hy < 0);
    SET_LDOUBLE_WORDS64(x, hx, lx);
    if (ix == 0)
        return __math_denorml(x);
    return x;
}

#if !defined(NEXTUP) && !defined(NEXTDOWN)
#ifdef __strong_reference
__strong_reference(nextafterl, nexttowardl);
#else
long double
nexttowardl(long double x, long double y)
{
    return nextafterl(x, y);
}
#endif
#endif
