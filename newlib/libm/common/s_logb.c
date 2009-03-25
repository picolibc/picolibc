/* 2009 for Newlib:  Sun's s_ilogb.c converted to be s_logb.c.  */
/* @(#)s_ilogb.c 5.1 93/09/24 */
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
       <<logb>>, <<logbf>>--get exponent of floating-point number
INDEX
	logb
INDEX
	logbf

ANSI_SYNOPSIS
	#include <math.h>
        double logb(double <[x]>);
        float logbf(float <[x]>);

DESCRIPTION
The <<logb>> functions extract the exponent of <[x]>, as a signed integer value
in floating-point format.  If <[x]> is subnormal it is treated as though it were
normalized; thus, for positive finite <[x]>,
@ifnottex
1 <= (<[x]> * FLT_RADIX to the power (-logb(<[x]>))) < FLT_RADIX.
@end ifnottex
@tex
$1 \leq ( x \cdot FLT\_RADIX ^ {-logb(x)} ) < FLT\_RADIX$.
@end tex
A domain error may occur if the argument is zero.
In this floating-point implementation, FLT_RADIX is 2.  Which also means
that for finite <[x]>, <<logb>>(<[x]>) = <<floor>>(<<log2>>(<<fabs>>(<[x]>))).

All nonzero, normal numbers can be described as
@ifnottex
<[m]> * 2**<[p]>, where 1.0 <= <[m]> < 2.0.
@end ifnottex
@tex
$m \cdot 2^p$, where $1.0 \leq m < 2.0$.
@end tex
The <<logb>> functions examine the argument <[x]>, and return <[p]>.
The <<frexp>> functions are similar to the <<logb>> functions, but
returning <[m]> adjusted to the interval [.5, 1) or 0, and <[p]>+1.

RETURNS
@comment Formatting note:  "$@" forces a new line
When <[x]> is:@*
+inf or -inf, +inf is returned;@*
NaN, NaN is returned;@*
0, -inf is returned, and the divide-by-zero exception is raised;@*
otherwise, the <<logb>> functions return the signed exponent of <[x]>.

PORTABILITY
ANSI C, POSIX

SEEALSO
frexp, ilogb
*/

/* double logb(double x)
 * return the binary exponent of non-zero x
 * logb(0) = -inf, raise divide-by-zero floating point exception
 * logb(+inf|-inf) = +inf (no signal is raised)
 * logb(NaN) = NaN (no signal is raised)
 * Per C99 recommendation, a NaN argument is returned unchanged.
 */

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

double
#ifdef __STDC__
logb(double x)
#else
logb(x)
double x;
#endif
{
	__int32_t hx,lx,ix;

	EXTRACT_WORDS(hx,lx,x);
	hx &= 0x7fffffff;		/* high |x| */
	if(hx<0x00100000) {		/* 0 or subnormal */
	    if((hx|lx)==0)  {
		double  xx;
		/* arg==0:  return -inf and raise divide-by-zero exception */
		INSERT_WORDS(xx,hx,lx);	/* +0.0 */
		return -1./xx;	/* logb(0) = -inf */
		}
	    else			/* subnormal x */
		if(hx==0) {
		    for (ix = -1043; lx>0; lx<<=1) ix -=1;
		} else {
		    for (ix = -1022,hx<<=11; hx>0; hx<<=1) ix -=1;
		}
	    return (double) ix;
	}
	else if (hx<0x7ff00000) return (hx>>20)-1023;	/* normal # */
	else if (hx>0x7ff00000 || lx)  return x;	/* x==NaN */
	else  return HUGE_VAL;	/* x==inf (+ or -) */
}

#endif /* _DOUBLE_IS_32BITS */
