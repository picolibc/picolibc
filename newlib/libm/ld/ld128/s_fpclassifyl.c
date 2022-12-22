/* Copyright (C) 2002, 2007 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

int
__fpclassifyl (long double x)
{
        int64_t hx;
        u_int64_t lx;

        GET_LDOUBLE_WORDS64(hx, lx, x);

        hx &= 0x7fffffffffffffffLL;

        if (hx == 0 && lx == 0)
                return FP_ZERO;
        else if (hx <= 0x0000ffffffffffffLL)
                /* zero is already handled above */
                return FP_SUBNORMAL;
        else if (hx <= 0x7ffeffffffffffffLL)
                return FP_NORMAL;
        else if (hx == 0x7fff000000000000LL && lx == 0)
                return FP_INFINITE;
        else
                return FP_NAN;
}

