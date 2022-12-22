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
nextafterl(long double x, long double y)
{
	int32_t hx,hy,ix,iy;
	u_int32_t lx,ly;
        int32_t esx,esy;

	GET_LDOUBLE_WORDS(esx,hx,lx,x);
	GET_LDOUBLE_WORDS(esy,hy,ly,y);
	ix = esx&0x7fff;		/* |x| */
	iy = esy&0x7fff;		/* |y| */

	if (((ix==0x7fff)&&(((hx&0x7fffffff)|lx)!=0)) ||   /* x is nan */
	    ((iy==0x7fff)&&(((hy&0x7fffffff)|ly)!=0)))     /* y is nan */
	   return x+y;
	if(x==y) return y;		/* x=y, return y */
	if((ix|hx|lx)==0) {			/* x == 0 */
            SET_LDOUBLE_WORDS(x,esy & 0x8000,hx,1);
            force_eval_long_double(x*x);
            return x;
	}
	if(esx<0x8000) {			/* x > 0 */
	    if(ix>iy||((ix==iy) && (hx>hy||((hx==hy)&&(lx>ly))))) {
	      /* x > y, x -= ulp */
		if(lx==0) {
		    if ((hx&0x7fffffff)==0) esx -= 1;
		    hx = (hx - 1) | (hx & 0x80000000);
		}
		lx -= 1;
	    } else {				/* x < y, x += ulp */
		lx += 1;
		if(lx==0) {
		    hx = (hx + 1) | (hx & 0x80000000);
		    if ((hx&0x7fffffff)==0) esx += 1;
		}
	    }
	} else {				/* x < 0 */
	    if(esy>=0||(ix>iy||((ix==iy)&&(hx>hy||((hx==hy)&&(lx>ly)))))){
	      /* x < y, x -= ulp */
		if(lx==0) {
		    if ((hx&0x7fffffff)==0) esx -= 1;
		    hx = (hx - 1) | (hx & 0x80000000);
		}
		lx -= 1;
	    } else {				/* x > y, x += ulp */
		lx += 1;
		if(lx==0) {
		    hx = (hx + 1) | (hx & 0x80000000);
		    if ((hx&0x7fffffff)==0) esx += 1;
		}
	    }
	}
	esy = esx&0x7fff;
	if(esy==0x7fff)
            return __math_oflowl(esx & 0x8000);		/* overflow  */
	SET_LDOUBLE_WORDS(x,esx,hx,lx);
	if(esy==0)
            return __math_denorml(x);
	return x;
}

#ifdef _HAVE_ALIAS_ATTRIBUTE
__strong_reference(nextafterl, nexttowardl);
#else
long double
nexttowardl(long double x, long double y)
{
    return nextafterl(x, y);
}
#endif
