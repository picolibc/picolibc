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

float nextafterf(float x, float y)
{
	__int32_t	hx,hy,ix,iy;

	GET_FLOAT_WORD(hx,x);
	GET_FLOAT_WORD(hy,y);
	ix = hx&0x7fffffff;		/* |x| */
	iy = hy&0x7fffffff;		/* |y| */

	if(FLT_UWORD_IS_NAN(ix) ||
	   FLT_UWORD_IS_NAN(iy))
	   return x+y;
	if(x==y) return y;		/* x=y, return y */
	if(FLT_UWORD_IS_ZERO(ix)) {		/* x == 0 */
	    SET_FLOAT_WORD(x,(hy&0x80000000)|FLT_UWORD_MIN);
	    force_eval_float(x*x);             /* raise underflow flag */
            return x;
	}
	if(hx>=0) {				/* x > 0 */
	    if(hx>hy) {				/* x > y, x -= ulp */
		hx -= 1;
	    } else {				/* x < y, x += ulp */
		hx += 1;
	    }
	} else {				/* x < 0 */
	    if(hy>=0||hx>hy){			/* x < y, x -= ulp */
		hx -= 1;
	    } else {				/* x > y, x += ulp */
		hx += 1;
	    }
	}
	hy = hx&0x7f800000;
	if(hy>FLT_UWORD_MAX) return check_oflowf(x+x);	/* overflow  */
	if(hy<0x00800000) {		/* underflow */
            force_eval_float(x*x);      /* raise underflow flag */
	}
	SET_FLOAT_WORD(x,hx);
	return check_uflowf(x);
}

#ifdef _DOUBLE_IS_32BITS

double nextafter(double x, double y)
{
	return (double) nextafterf((float) x, (float) y);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
