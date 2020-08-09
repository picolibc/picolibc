
/* @(#)w_sqrt.c 5.1 93/09/24 */
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
	<<sqrt>>, <<sqrtf>>---positive square root

INDEX
	sqrt
INDEX
	sqrtf

SYNOPSIS
	#include <math.h>
	double sqrt(double <[x]>);
	float  sqrtf(float <[x]>);

DESCRIPTION
	<<sqrt>> computes the positive square root of the argument.

RETURNS
	On success, the square root is returned. If <[x]> is real and
	positive, then the result is positive.  If <[x]> is real and
	negative, the global value <<errno>> is set to <<EDOM>> (domain error).


PORTABILITY
	<<sqrt>> is ANSI C.  <<sqrtf>> is an extension.
*/

/* 
 * wrapper sqrt(x)
 */

#include "fdlibm.h"
#include <errno.h>

#if !defined(_IEEE_LIBM) || !defined(HAVE_ALIAS_ATTRIBUTE)
#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double sqrt(double x)		/* wrapper sqrt */
#else
	double sqrt(x)			/* wrapper sqrt */
	double x;
#endif
{
	double z;
	z = __ieee754_sqrt(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(x<0.0)
	    errno = EDOM;
	return z;
}

#endif /* defined(_DOUBLE_IS_32BITS) */
#endif /* defined(_IEEE_LIBM) */
