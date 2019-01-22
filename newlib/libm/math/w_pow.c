

/* @(#)w_pow.c 5.2 93/10/01 */
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
	<<pow>>, <<powf>>---x to the power y
INDEX
	pow
INDEX
	powf


SYNOPSIS
	#include <math.h>
	double pow(double <[x]>, double <[y]>);
	float powf(float <[x]>, float <[y]>);

DESCRIPTION
	<<pow>> and <<powf>> calculate <[x]> raised to the exponent <[y]>.
	@tex
	(That is, $x^y$.)
	@end tex

RETURNS
	On success, <<pow>> and <<powf>> return the value calculated.

	When the argument values would produce overflow, <<pow>>
	returns <<HUGE_VAL>> and set <<errno>> to <<ERANGE>>.  If the
	argument <[x]> passed to <<pow>> or <<powf>> is a negative
	noninteger, and <[y]> is also not an integer, then <<errno>>
	is set to <<EDOM>>.  If <[x]> and <[y]> are both 0, then
	<<pow>> and <<powf>> return <<1>>.

PORTABILITY
	<<pow>> is ANSI C. <<powf>> is an extension.  */

/* 
 * wrapper pow(x,y) return x**y
 */

#include "fdlibm.h"
#if __OBSOLETE_MATH
#include <errno.h>

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double pow(double x, double y)	/* wrapper pow */
#else
	double pow(x,y)			/* wrapper pow */
	double x,y;
#endif
{
#ifdef _IEEE_LIBM
	return  __ieee754_pow(x,y);
#else
	double z;
	z=__ieee754_pow(x,y);
	if(_LIB_VERSION == _IEEE_|| isnan(y)) return z;
	if(isnan(x)) {
	    if(y==0.0) { 
		/* pow(NaN,0.0) */
		/* Not an error.  */
                return 1.0;
	    } else 
		return z;
	}
	if(x==0.0){ 
	    if(y==0.0) {
		/* pow(0.0,0.0) */
		/* Not an error.  */
		return 1.0;
	    }
	    if(finite(y)&&y<0.0) {
		/* 0**neg */
		errno = EDOM;
		return -HUGE_VAL;
	    }
	    return z;
	}
	if(!finite(z)) {
	    if(finite(x)&&finite(y)) {
	        if(isnan(z)) {
		    /* neg**non-integral */
		    errno = EDOM;
		    return 0.0/0.0;
	        } else {
		    /* pow(x,y) overflow */
		    errno = ERANGE;
		    if(x<0.0&&rint(y)!=y)
		      return -HUGE_VAL;
		    return HUGE_VAL;
                }
	    }
	} 
	if(z==0.0&&finite(x)&&finite(y)) {
	    /* pow(x,y) underflow */
	    errno = ERANGE;
	    return 0.0;
        } 
	return z;
#endif
}

#endif /* defined(_DOUBLE_IS_32BITS) */
#endif /* __OBSOLETE_MATH */













