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

#include <stdio.h>

float
nexttowardf(float x, long double y)
{
	int32_t hx,ix;
	int64_t hy,iy;
	u_int64_t ly;

	GET_FLOAT_WORD(hx,x);
	GET_LDOUBLE_WORDS64(hy,ly,y);
	ix = hx&0x7fffffff;		/* |x| */
	iy = hy&0x7fffffffffffffffLL;	/* |y| */

	if(ix>0x7f800000) {       /* x is nan */
            force_eval_long_double(y+y);
            return x + x;
        }
        if((iy>=0x7fff000000000000LL)&&((iy-0x7fff000000000000LL)|ly)!=0) { /* y is nan */
            return (float) (y + y);
        }
	if((long double) x==y) return y;	/* x=y, return y */
	if(ix==0) {				/* x == 0 */
	    SET_FLOAT_WORD(x,(u_int32_t)((hy>>32)&0x80000000)|1);/* return +-minsub*/
            force_eval_float(x*x);
	    return x;
	}
	if(hx>=0) {				/* x > 0 */
	    if((long double) x > y) {           /* x > y, x -= ulp */
		hx -= 1;
	    } else {				/* x < y, x += ulp */
		hx += 1;
            }
	} else {				/* x < 0 */
	    if((long double) x < y) {           /* x < y, x -= ulp */
		hx -= 1;
	    } else {				/* x > y, x += ulp */
		hx += 1;
            }
	}
	ix = hx&0x7f800000;
	if(ix>=0x7f800000)
            return __math_oflowf(hx<0);
	SET_FLOAT_WORD(x,hx);
	if(ix<0x00800000)
            return __math_denormf(x);
	return x;
}
