/* @(#)s_scalbn.c 5.1 93/09/24 */
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
 * scalbn (double x, int n)
 * scalbn(x,n) returns x* 2**n  computed by  exponent
 * manipulation rather than by actually performing an
 * exponentiation or a multiplication.
 */

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

static const __float64
two54   =  _F_64(1.80143985094819840000e+16), /* 0x43500000, 0x00000000 */
twom54  =  _F_64(5.55111512312578270212e-17); /* 0x3C900000, 0x00000000 */

__float64
scalbln64 (__float64 x, long int n)
{
	__int32_t hx,lx;
        long int k;
	EXTRACT_WORDS(hx,lx,x);
        k = (hx&0x7ff00000)>>20;		/* extract exponent */
        if (k==0) {				/* 0 or subnormal x */
            if ((lx|(hx&0x7fffffff))==0) return x; /* +-0 */
	    x *= two54;
	    GET_HIGH_WORD(hx,x);
	    k = ((hx&0x7ff00000)>>20) - 54;
            if (n< -50000) return __math_uflow(hx < 0); /*underflow*/
	    }
        if (k==0x7ff) return x+x;		/* NaN or Inf */
        k = k+n;
        if (n> 50000 || k >  0x7fe)
            return __math_oflow(hx<0);          /* overflow  */
        if (k > 0) 				/* normal result */
	    {SET_HIGH_WORD(x,(hx&0x800fffff)|(k<<20)); return x;}
        if (k <= -54)
            return __math_uflow(hx < 0); 	/*underflow*/
        k += 54;				/* subnormal result */
	SET_HIGH_WORD(x,(hx&0x800fffff)|(k<<20));
        return check_uflow(x*twom54);
}

_MATH_ALIAS_d_dj(scalbln)

#endif /* _NEED_FLOAT64 */
