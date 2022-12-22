
/* @(#)s_rint.c 5.1 93/09/24 */
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
FUNCTION
<<rint>>, <<rintf>>---round to integer
INDEX
	rint
INDEX
	rintf

SYNOPSIS
	#include <math.h>
	double rint(double <[x]>);
	float rintf(float <[x]>);

DESCRIPTION
	The <<rint>> functions round their argument to an integer value in
	floating-point format, using the current rounding direction.  They
	raise the "inexact" floating-point exception if the result differs
	in value from the argument.  See the <<nearbyint>> functions for the
	same function with the "inexact" floating-point exception never being
	raised.  Newlib does not directly support floating-point exceptions.
	The <<rint>> functions are written so that the "inexact" exception is
	raised in hardware implementations that support it, even though Newlib
	does not provide access.

RETURNS
<[x]> rounded to an integral value, using the current rounding direction.

PORTABILITY
ANSI C, POSIX

SEEALSO
<<nearbyint>>, <<round>>

*/

/*
 * rint(x)
 * Return x rounded to integral value according to the prevailing
 * rounding mode.
 * Method:
 *	Using floating addition.
 *	Whenever a fraction is present, if the second or any following bit after
 *	the radix point is set, limit to the second radix point to avoid
 *	possible double rounding in the TWO52 +- steps (in case guard bits are
 *	used).  Specifically, if have any, chop off bits past the 2nd place and
 *	set the second place.
 *	(e.g.	2.0625=0b10.0001 => 0b10.01=2.25;
 *		2.3125=0b10.011  => 0b10.01=2.25;
 *		1.5625= 0b1.1001 =>  0b1.11=1.75;
 *		1.9375= 0b1.1111 =>  0b1.11=1.75.
 *	Pseudo-code:  if(x.frac & ~0b0.10) x.frac = (x.frac & 0b0.11) | 0b0.01;).
 * Exception:
 *	Inexact flag raised if x not equal to rint(x).
 */

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

#ifdef __STDC__
static const __float64
#else
static __float64 
#endif
TWO52[2]={
    _F_64(4.50359962737049600000e+15), /* 0x43300000, 0x00000000 */
    _F_64(-4.50359962737049600000e+15), /* 0xC3300000, 0x00000000 */
};

__float64
rint64(__float64 x)
{
	__int32_t i0,j0,sx;
	__uint32_t i,i1;
	__float64 t;
	volatile __float64 w;
	EXTRACT_WORDS(i0,i1,x);
	sx = (i0>>31)&1;		/* sign */
	j0 = ((i0>>20)&0x7ff)-0x3ff;	/* exponent */
	if(j0<20) {			/* no integral bits in LS part */
	    if(j0<0) { 			/* x is fractional or 0 */
		if(((i0&0x7fffffff)|i1)==0) return x;	/* x == 0 */
		i1 |= (i0&0x0fffff);
		i0 &= 0xfffe0000;
		i0 |= ((i1|-i1)>>12)&0x80000;
		SET_HIGH_WORD(x,i0);
	        w = TWO52[sx]+x;
	        t =  w-TWO52[sx];
		GET_HIGH_WORD(i0,t);
		SET_HIGH_WORD(t,(i0&0x7fffffff)|(sx<<31));
	        return t;
	    } else {			/* x has integer and maybe fraction */
		i = (0x000fffff)>>j0;
		if(((i0&i)|i1)==0) return x; /* x is integral */
		i>>=1;
		if(((i0&i)|i1)!=0) {
		    /* 2nd or any later bit after radix is set */
		    if(j0==19) i1 = 0x80000000; else i1 = 0;
		    i0 = (i0&(~i))|((0x40000)>>j0);
		}
	    }
	} else if (j0>51) {
            /*
             * Use barrier to avoid overflow on clang which would
             * otherwise always do this add on arm and use a
             * conditional move instead of a branch for the if
             */
            if (j0 == 0x400)
                return opt_barrier_double(x+x);
            return x;
	} else {
	    i = ((__uint32_t)(0xffffffff))>>(j0-20);
	    if((i1&i)==0) return x;	/* x is integral */
	    i>>=1;
	    if((i1&i)!=0) i1 = (i1&(~i))|((0x40000000)>>(j0-20));
	}
	INSERT_WORDS(x,i0,i1);
	w = TWO52[sx]+x;
	return w-TWO52[sx];
}

_MATH_ALIAS_d_d(rint)

#endif /* _NEED_FLOAT64 */
