
/* @(#)s_atan.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 */

/*
FUNCTION
        <<atan>>, <<atanf>>---arc tangent

INDEX
   atan
INDEX
   atanf

SYNOPSIS
        #include <math.h>
        double atan(double <[x]>);
        float atanf(float <[x]>);

DESCRIPTION

<<atan>> computes the inverse tangent (arc tangent) of the input value.

<<atanf>> is identical to <<atan>>, save that it operates on <<floats>>.

RETURNS
@ifnottex
<<atan>> returns a value in radians, in the range of -pi/2 to pi/2.
@end ifnottex
@tex
<<atan>> returns a value in radians, in the range of $-\pi/2$ to $\pi/2$.
@end tex

PORTABILITY
<<atan>> is ANSI C.  <<atanf>> is an extension.

*/

/* atan(x)
 * Method
 *   1. Reduce x to positive by atan(x) = -atan(-x).
 *   2. According to the integer k=4t+0.25 chopped, t=x, the argument
 *      is further reduced to one of the following intervals and the
 *      arctangent of t is evaluated by the corresponding formula:
 *
 *      [0,7/16]      atan(x) = t-t^3*(a1+t^2*(a2+...(a10+t^2*a11)...)
 *      [7/16,11/16]  atan(x) = atan(1/2) + atan( (t-0.5)/(1+t/2) )
 *      [11/16.19/16] atan(x) = atan( 1 ) + atan( (t-1)/(1+t) )
 *      [19/16,39/16] atan(x) = atan(3/2) + atan( (t-1.5)/(1+1.5t) )
 *      [39/16,INF]   atan(x) = atan(INF) + atan( -1/t )
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

static const __float64 atanhi[] = {
    _F_64(4.63647609000806093515e-01), /* atan(0.5)hi 0x3FDDAC67, 0x0561BB4F */
    _F_64(7.85398163397448278999e-01), /* atan(1.0)hi 0x3FE921FB, 0x54442D18 */
    _F_64(9.82793723247329054082e-01), /* atan(1.5)hi 0x3FEF730B, 0xD281F69B */
    _F_64(1.57079632679489655800e+00), /* atan(inf)hi 0x3FF921FB, 0x54442D18 */
};

static const __float64 atanlo[] = {
    _F_64(2.26987774529616870924e-17), /* atan(0.5)lo 0x3C7A2B7F, 0x222F65E2 */
    _F_64(3.06161699786838301793e-17), /* atan(1.0)lo 0x3C81A626, 0x33145C07 */
    _F_64(1.39033110312309984516e-17), /* atan(1.5)lo 0x3C700788, 0x7AF0CBBD */
    _F_64(6.12323399573676603587e-17), /* atan(inf)lo 0x3C91A626, 0x33145C07 */
};

static const __float64 aT[] = {
    _F_64(3.33333333333329318027e-01), /* 0x3FD55555, 0x5555550D */
    _F_64(-1.99999999998764832476e-01), /* 0xBFC99999, 0x9998EBC4 */
    _F_64(1.42857142725034663711e-01), /* 0x3FC24924, 0x920083FF */
    _F_64(-1.11111104054623557880e-01), /* 0xBFBC71C6, 0xFE231671 */
    _F_64(9.09088713343650656196e-02), /* 0x3FB745CD, 0xC54C206E */
    _F_64(-7.69187620504482999495e-02), /* 0xBFB3B0F2, 0xAF749A6D */
    _F_64(6.66107313738753120669e-02), /* 0x3FB10D66, 0xA0D03D51 */
    _F_64(-5.83357013379057348645e-02), /* 0xBFADDE2D, 0x52DEFD9A */
    _F_64(4.97687799461593236017e-02), /* 0x3FA97B4B, 0x24760DEB */
    _F_64(-3.65315727442169155270e-02), /* 0xBFA2B444, 0x2C6A6C2F */
    _F_64(1.62858201153657823623e-02), /* 0x3F90AD3A, 0xE322DA11 */
};

static const __float64 one = _F_64(1.0);

__float64
atan64(__float64 x)
{
    __float64 w, s1, s2, z;
    __int32_t ix, hx, id;

    GET_HIGH_WORD(hx, x);
    ix = hx & 0x7fffffff;
    if (ix >= 0x44100000) { /* if |x| >= 2^66 */
        __uint32_t low;
        GET_LOW_WORD(low, x);
        if (ix > 0x7ff00000 || (ix == 0x7ff00000 && (low != 0)))
            return x + x; /* NaN */
        if (hx > 0)
            return atanhi[3] + atanlo[3];
        else
            return -atanhi[3] - atanlo[3];
    }
    if (ix < 0x3fdc0000) { /* |x| < 0.4375 */
        if (ix < 0x3e200000) /* |x| < 2^-29 */
            return __math_inexact64(x);
        id = -1;
    } else {
        x = fabs64(x);
        if (ix < 0x3ff30000) { /* |x| < 1.1875 */
            if (ix < 0x3fe60000) { /* 7/16 <=|x|<11/16 */
                id = 0;
                x = (_F_64(2.0) * x - one) / (_F_64(2.0) + x);
            } else { /* 11/16<=|x|< 19/16 */
                id = 1;
                x = (x - one) / (x + one);
            }
        } else {
            if (ix < 0x40038000) { /* |x| < 2.4375 */
                id = 2;
                x = (x - _F_64(1.5)) / (one + 1.5 * x);
            } else { /* 2.4375 <= |x| < 2^66 */
                id = 3;
                x = _F_64(-1.0) / x;
            }
        }
    }
    /* end of argument reduction */
    z = x * x;
    w = z * z;
    /* break sum from i=0 to 10 aT[i]z**(i+1) into odd and even poly */
    s1 = z *
         (aT[0] +
          w * (aT[2] + w * (aT[4] + w * (aT[6] + w * (aT[8] + w * aT[10])))));
    s2 = w * (aT[1] + w * (aT[3] + w * (aT[5] + w * (aT[7] + w * aT[9]))));
    if (id < 0)
        return x - x * (s1 + s2);
    else {
        z = atanhi[id] - ((x * (s1 + s2) - atanlo[id]) - x);
        return (hx < 0) ? -z : z;
    }
}

_MATH_ALIAS_d_d(atan)

#endif /* _NEED_FLOAT64 */
