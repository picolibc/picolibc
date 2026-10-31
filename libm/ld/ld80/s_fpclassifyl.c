/* Copyright (C) 2002, 2007 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

int
__fpclassifyl(long double x)
{
    u_int32_t hx, lx, esx;

    GET_LDOUBLE_WORDS(esx, hx, lx, x);

    esx &= 0x7fff;
    if (esx == 0 && hx == 0 && lx == 0)
        return FP_ZERO;
    else if (esx == 0)
        /* zero is already handled above */
        return FP_SUBNORMAL;
#ifndef __m68k__
    /*
     * x86 treats values without the explicit integer bit as invalid;
     * m68k accepts them, and its infinity has the bit clear
     */
    else if ((hx & 0x80000000) == 0)
        return FP_NAN;
#endif
    else if (esx < 0x7fff)
        return FP_NORMAL;
    else if (hx == LDBL_NBIT_INF && lx == 0)
        return FP_INFINITE;
    else
        return FP_NAN;
}
