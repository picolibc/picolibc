
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
FUNCTION
<<scalbn>>, <<scalbnf>>, <<scalbln>>, <<scalblnf>>---scale by power of FLT_RADIX (=2)
INDEX
	scalbn
INDEX
	scalbnf
INDEX
	scalbln
INDEX
	scalblnf

SYNOPSIS
	#include <math.h>
	double scalbn(double <[x]>, int <[n]>);
	float scalbnf(float <[x]>, int <[n]>);
	double scalbln(double <[x]>, long int <[n]>);
	float scalblnf(float <[x]>, long int <[n]>);

DESCRIPTION
The <<scalbn>> and <<scalbln>> functions compute 
	@ifnottex
	<[x]> times FLT_RADIX to the power <[n]>.
	@end ifnottex
	@tex
	$x \cdot FLT\_RADIX^n$.
	@end tex
efficiently.  The result is computed by manipulating the exponent, rather than
by actually performing an exponentiation or multiplication.  In this
floating-point implementation FLT_RADIX=2, which makes the <<scalbn>>
functions equivalent to the <<ldexp>> functions.

RETURNS
<[x]> times 2 to the power <[n]>.  A range error may occur.

PORTABILITY
ANSI C, POSIX

SEEALSO
<<ldexp>>

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
scalbn64(__float64 x, int n)
{
	__int32_t  k,hx,lx;
	EXTRACT_WORDS(hx,lx,x);
        k = (hx&0x7ff00000)>>20;		/* extract exponent */
        if (k==0) {				/* 0 or subnormal x */
            if ((lx|(hx&0x7fffffff))==0) return x; /* +-0 */
	    x *= two54; 
	    GET_HIGH_WORD(hx,x);
	    k = ((hx&0x7ff00000)>>20) - 54; 
#if __SIZEOF_INT__ > 2
            if (n< -50000) return __math_uflow(hx<0); 	/*underflow*/
#endif
	    }
        if (k==0x7ff) return x+x;		/* NaN or Inf */
#if __SIZEOF_INT__ > 2
        if (n > 50000) 	/* in case integer overflow in n+k */
            return __math_oflow(hx<0);	        /*overflow*/
#endif
        k = k+n; 
        if (k >  0x7fe) return __math_oflow(hx<0); /* overflow  */
        if (k > 0) 				/* normal result */
	    {SET_HIGH_WORD(x,(hx&0x800fffff)|(k<<20)); return x;}
        if (k <= -54)
	    return __math_uflow(hx<0); 	        /*underflow*/
        k += 54;				/* subnormal result */
	SET_HIGH_WORD(x,(hx&0x800fffff)|(k<<20));
        return check_uflow(x*twom54);
}

#if defined(_HAVE_ALIAS_ATTRIBUTE)
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(scalbn64, ldexp64);
#else

__float64
ldexp64(__float64 value, int exp)
{
    return scalbn64(value, exp);
}

#endif

_MATH_ALIAS_d_di(scalbn)
_MATH_ALIAS_d_di(ldexp)

#endif /* _NEED_FLOAT64 */
