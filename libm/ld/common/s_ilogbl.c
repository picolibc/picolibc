/*
 * From: @(#)s_ilogb.c 5.1 93/09/24
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
#include <limits.h>

#ifdef LLOGB
#define NAME    llogbl
#define TYPE    long int
#define LOGB0   FP_LLOGB0
#define LOGBNAN FP_LLOGBNAN
#define LOGBINF LONG_MAX
#else
#define NAME    ilogbl
#define TYPE    int
#define LOGB0   FP_ILOGB0
#define LOGBNAN FP_ILOGBNAN
#define LOGBINF INT_MAX
#endif

TYPE
NAME(long double x)
{
    union IEEEl2bits u;
    uint64_t         m;
    int              b;

    u.e = x;
    if (u.bits.exp == 0) {
        if ((u.bits.manl | u.bits.manh) == 0) {
            (void)__math_invalidl(x);
            return LOGB0;
        }
        /* denormalized */
#ifdef LDBL_MANL_SIZE
        if (u.bits.manh == 0) {
            m = 1llu << (LDBL_MANL_SIZE - 1);
            for (b = LDBL_MANH_SIZE; !(u.bits.manl & m); m >>= 1)
                b++;
        } else
#endif
        {
            m = 1llu << (LDBL_MANH_SIZE - 1);
            for (b = 0; !(u.bits.manh & m); m >>= 1)
                b++;
        }
#ifdef LDBL_IMPLICIT_NBIT
        b++;
#endif
        return (TYPE)(LDBL_MIN_EXP - b - 1);
    } else if (u.bits.exp < (LDBL_MAX_EXP << 1) - 1)
        return (TYPE)(u.bits.exp - LDBL_MAX_EXP + 1);
    else if (u.bits.manl != 0 || u.bits.manh != LDBL_NBIT_INF) {
        (void)__math_invalidl(0.0L);
        return LOGBNAN;
    } else {
        (void)__math_invalidl(0.0L);
        return LOGBINF;
    }
}
