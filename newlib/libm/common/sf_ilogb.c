/* sf_ilogb.c -- float version of s_ilogb.c.
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

#include <limits.h>
#include "fdlibm.h"

int ilogbf(float x)
{
	__int32_t hx,ix;

	GET_FLOAT_WORD(hx,x);
	hx &= 0x7fffffff;
	if(FLT_UWORD_IS_ZERO(hx)) {
            (void) __math_invalidf(0.0);
	    return FP_ILOGB0;	/* ilogb(0) = special case error */
        }
	if(FLT_UWORD_IS_SUBNORMAL(hx)) {
	    for (ix = -126,hx<<=8; hx>0; hx<<=1) ix -=1;
	    return ix;
	}
#if FP_ILOGBNAN != INT_MAX
	else if (FLT_UWORD_IS_NAN(hx)) {
            (void) __math_invalidf(0.0);
            return FP_ILOGBNAN;	/* NAN */
        }
#endif
	else if (!FLT_UWORD_IS_FINITE(hx)) {
            (void) __math_invalidf(0.0);
            return INT_MAX;
        }
	else return (hx>>23)-127;
}

_MATH_ALIAS_i_f(ilogb)
