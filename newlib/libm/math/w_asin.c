
/* @(#)w_asin.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

/*
FUNCTION
        <<asin>>, <<asinf>>---arc sine

INDEX
   asin
INDEX
   asinf

SYNOPSIS
        #include <math.h>
        double asin(double <[x]>);
        float asinf(float <[x]>);

DESCRIPTION

<<asin>> computes the inverse sine (arc sine) of the argument <[x]>.
Arguments to <<asin>> must be in the range @minus{}1 to 1.

<<asinf>> is identical to <<asin>>, other than taking and
returning floats.

RETURNS
@ifnottex
<<asin>> returns values in radians, in the range of -pi/2 to pi/2.
@end ifnottex
@tex
<<asin>> returns values in radians, in the range of $-\pi/2$ to $\pi/2$.
@end tex

If <[x]> is not in the range @minus{}1 to 1, <<asin>> and <<asinf>>
return NaN (not a number), and the global variable <<errno>> is set to
<<EDOM>>.

QUICKREF
 ansi posix rentrant
 asin	 y,y,m
 asinf   n,n,m

MATHREF  
 asin,  -1<=arg<=1, asin(arg),,,
 asin,  NAN,  arg,EDOM, DOMAIN

MATHREF  
 asinf,  -1<=arg<=1, asin(arg),,,
 asinf,  NAN,  arg,EDOM, DOMAIN 


*/

/* 
 * wrapper asin(x)
 */


#include "fdlibm.h"
#include <errno.h>

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double asin(double x)		/* wrapper asin */
#else
	double asin(x)			/* wrapper asin */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_asin(x);
#else
	double z;
	z = __ieee754_asin(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(fabs(x)>1.0) {
	    /* asin(|x|>1) */
	    errno = EDOM;
	    return nan("");
	} else
	    return z;
#endif
}

#endif /* defined(_DOUBLE_IS_32BITS) */
