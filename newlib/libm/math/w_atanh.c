
/* @(#)w_atanh.c 5.1 93/09/24 */
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
	<<atanh>>, <<atanhf>>---inverse hyperbolic tangent 

INDEX
	atanh
INDEX
	atanhf

SYNOPSIS
	#include <math.h>
	double atanh(double <[x]>);
	float atanhf(float <[x]>);

DESCRIPTION
	<<atanh>> calculates the inverse hyperbolic tangent of <[x]>.

	<<atanhf>> is identical, other than taking and returning
	<<float>> values.

RETURNS
	<<atanh>> and <<atanhf>> return the calculated value.

	If 
	@ifnottex
	|<[x]>|
	@end ifnottex
	@tex
	$|x|$
	@end tex
	is greater than 1, the global <<errno>> is set to <<EDOM>> and
	the result is a NaN.  A <<DOMAIN error>> is reported.

	If 
	@ifnottex
	|<[x]>|
	@end ifnottex
	@tex
	$|x|$
	@end tex
	is 1, the global <<errno>> is set to <<EDOM>>; and the result is 
	infinity with the same sign as <<x>>.  A <<SING error>> is reported.

PORTABILITY
	Neither <<atanh>> nor <<atanhf>> are ANSI C.

QUICKREF
	atanh - pure
	atanhf - pure


*/

/* 
 * wrapper atanh(x)
 */

#include "fdlibm.h"
#include <errno.h>

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double atanh(double x)		/* wrapper atanh */
#else
	double atanh(x)			/* wrapper atanh */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_atanh(x);
#else
	double z,y;
	z = __ieee754_atanh(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	y = fabs(x);
	if(y>=1.0) {
	    if(y>1.0) {
		/* atanh(|x|>1) */
		errno = EDOM;
		return 0.0/0.0;
	    } else { 
		/* atanh(|x|=1) */
		errno = EDOM;
		return x/0.0;	/* sign(x)*inf */
	    }
	} else
	    return z;
#endif
}

#endif /* defined(_DOUBLE_IS_32BITS) */




