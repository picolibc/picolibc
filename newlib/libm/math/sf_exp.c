/* ef_exp.c -- float version of e_exp.c.
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
#include "math_config.h"

#if __OBSOLETE_MATH_FLOAT
#ifdef __v810__
#define const
#endif

static const float
one	= 1.0,
halF[2]	= {0.5,-0.5,},
huge	= 1.0e+30,
twom100 = 7.8886090522e-31,      /* 2**-100=0x0d800000 */
ln2HI[2]   ={ 6.9313812256e-01,		/* 0x3f317180 */
	     -6.9313812256e-01,},	/* 0xbf317180 */
ln2LO[2]   ={ 9.0580006145e-06,  	/* 0x3717f7d1 */
	     -9.0580006145e-06,},	/* 0xb717f7d1 */
invln2 =  1.4426950216e+00, 		/* 0x3fb8aa3b */
P1   =  1.6666667163e-01, /* 0x3e2aaaab */
P2   = -2.7777778450e-03, /* 0xbb360b61 */
P3   =  6.6137559770e-05, /* 0x388ab355 */
P4   = -1.6533901999e-06, /* 0xb5ddea0e */
P5   =  4.1381369442e-08; /* 0x3331bb4c */

float
expf(float x) /* default IEEE double exp */
{
    float y, hi, lo, c, t;
    __int32_t k = 0, xsb, sx;
    __uint32_t hx;

    GET_FLOAT_WORD(sx, x);
    xsb = (sx >> 31) & 1; /* sign bit of x */
    hx = sx & 0x7fffffff; /* high word of |x| */

    /* filter out non-finite argument */
    if (FLT_UWORD_IS_NAN(hx))
        return x + x; /* NaN */
    if (FLT_UWORD_IS_INFINITE(hx))
        return (xsb == 0) ? x : 0.0f; /* exp(+-inf)={inf,0} */
    if (sx > FLT_UWORD_LOG_MAX)
        return __math_oflowf(0); /* overflow */
    if (sx < 0 && hx > FLT_UWORD_LOG_MIN)
        return __math_uflowf(0); /* underflow */

    /* argument reduction */
    if (hx > 0x3eb17218) { /* if  |x| > 0.5 ln2 */
        if (hx < 0x3F851592) { /* and |x| < 1.5 ln2 */
            hi = x - ln2HI[xsb];
            lo = ln2LO[xsb];
            k = 1 - xsb - xsb;
        } else {
            k = invln2 * x + halF[xsb];
            t = k;
            hi = x - t * ln2HI[0]; /* t*ln2HI is exact here */
            lo = t * ln2LO[0];
        }
        x = hi - lo;
    } else if (hx < 0x34000000) { /* when |x|<2**-23 */
        if (huge + x > one)
            return one + x; /* trigger inexact */
    }

    /* x is now in primary range */
    t = x * x;
    c = x - t * (P1 + t * (P2 + t * (P3 + t * (P4 + t * P5))));
    if (k == 0)
        return one - ((x * c) / (c - 2.0f) - x);
    else
        y = one - ((lo - (x * c) / (2.0f - c)) - hi);
    if (k >= -125) {
        __uint32_t hy;
        GET_FLOAT_WORD(hy, y);
        SET_FLOAT_WORD(y, hy + (k << 23)); /* add k to y's exponent */
        return y;
    } else {
        __uint32_t hy;
        GET_FLOAT_WORD(hy, y);
        SET_FLOAT_WORD(y, hy + ((k + 100) << 23)); /* add k to y's exponent */
        return y * twom100;
    }
}

_MATH_ALIAS_f_f(exp)

#else
#include "../common/sf_exp.c"
#endif /* __OBSOLETE_MATH_FLOAT */
