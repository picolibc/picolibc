
/* @(#)w_exp.c 5.1 93/09/24 */
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
	<<exp>>, <<expf>>---exponential
INDEX
	exp
INDEX
	expf

SYNOPSIS
	#include <math.h>
	double exp(double <[x]>);
	float expf(float <[x]>);

DESCRIPTION
	<<exp>> and <<expf>> calculate the exponential of <[x]>, that is, 
	@ifnottex
	e raised to the power <[x]> (where e
	@end ifnottex
	@tex
	$e^x$ (where $e$
	@end tex
	is the base of the natural system of logarithms, approximately 2.71828).

RETURNS
	On success, <<exp>> and <<expf>> return the calculated value.
	If the result underflows, the returned value is <<0>>.  If the
	result overflows, the returned value is <<HUGE_VAL>>.  In
	either case, <<errno>> is set to <<ERANGE>>.

PORTABILITY
	<<exp>> is ANSI C.  <<expf>> is an extension.

*/

/* 
 * wrapper exp(x)
 */

#include "fdlibm.h"
#if __OBSOLETE_MATH
#include <errno.h>

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
static const double
#else
static double
#endif
o_threshold=  7.09782712893383973096e+02,  /* 0x40862E42, 0xFEFA39EF */
u_threshold= -7.45133219101941108420e+02;  /* 0xc0874910, 0xD52D3051 */

#ifdef __STDC__
	double exp(double x)		/* wrapper exp */
#else
	double exp(x)			/* wrapper exp */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_exp(x);
#else
	double z;
	z = __ieee754_exp(x);
	if(_LIB_VERSION == _IEEE_) return z;
	if(finite(x)) {
	    if(x>o_threshold) {
		/* exp(finite) overflow */
		errno = ERANGE;
		return HUGE_VAL;
	    } else if(x<u_threshold) {
		/* exp(finite) underflow */
		errno = ERANGE;
		return 0.0;
	    } 
	} 
	return z;
#endif
}

#endif /* defined(_DOUBLE_IS_32BITS) */
#endif /* __OBSOLETE_MATH */
