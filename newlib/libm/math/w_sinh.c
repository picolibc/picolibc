
/* @(#)w_sinh.c 5.1 93/09/24 */
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
        <<sinh>>, <<sinhf>>---hyperbolic sine

INDEX
	sinh
INDEX
	sinhf

SYNOPSIS
        #include <math.h>
        double sinh(double <[x]>);
        float  sinhf(float <[x]>);

DESCRIPTION
	<<sinh>> computes the hyperbolic sine of the argument <[x]>.
	Angles are specified in radians.   <<sinh>>(<[x]>) is defined as 
	@ifnottex
	. (exp(<[x]>) - exp(-<[x]>))/2
	@end ifnottex
	@tex
	$${e^x - e^{-x}}\over 2$$
	@end tex

	<<sinhf>> is identical, save that it takes and returns <<float>> values.

RETURNS
	The hyperbolic sine of <[x]> is returned.  

	When the correct result is too large to be representable (an
	overflow),  <<sinh>> returns <<HUGE_VAL>> with the
	appropriate sign, and sets the global value <<errno>> to
	<<ERANGE>>. 

PORTABILITY
	<<sinh>> is ANSI C.  
	<<sinhf>> is an extension.

QUICKREF
	sinh ansi pure
	sinhf - pure
*/

/* 
 * wrapper sinh(x)
 */

#include "fdlibm.h"
#include <errno.h>

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double sinh(double x)		/* wrapper sinh */
#else
	double sinh(x)			/* wrapper sinh */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_sinh(x);
#else
	double z; 
	z = __ieee754_sinh(x);
	if(_LIB_VERSION == _IEEE_) return z;
	if(!finite(z)&&finite(x)) {
	    /* sinh(finite) overflow */
	    errno = ERANGE;
	    return ((x>0.0) ? HUGE_VAL : -HUGE_VAL);
	} else
	    return z;
#endif
}

#endif /* defined(_DOUBLE_IS_32BITS) */
