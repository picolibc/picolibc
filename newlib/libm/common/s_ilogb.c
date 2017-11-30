
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
       <<ilogb>>, <<ilogbf>>---get exponent of floating-point number
INDEX
	ilogb
INDEX
	ilogbf

SYNOPSIS
	#include <math.h>
        int ilogb(double <[val]>);
        int ilogbf(float <[val]>);

DESCRIPTION

	All nonzero, normal numbers can be described as <[m]> *
	2**<[p]>.  <<ilogb>> and <<ilogbf>> examine the argument
	<[val]>, and return <[p]>.  The functions <<frexp>> and
	<<frexpf>> are similar to <<ilogb>> and <<ilogbf>>, but also
	return <[m]>.

RETURNS

<<ilogb>> and <<ilogbf>> return the power of two used to form the
floating-point argument.
If <[val]> is <<0>>, they return <<FP_ILOGB0>>.
If <[val]> is infinite, they return <<INT_MAX>>.
If <[val]> is NaN, they return <<FP_ILOGBNAN>>.
(<<FP_ILOGB0>> and <<FP_ILOGBNAN>> are defined in math.h, but in turn are
defined as INT_MIN or INT_MAX from limits.h.  The value of FP_ILOGB0 may be
either INT_MIN or -INT_MAX.  The value of FP_ILOGBNAN may be either INT_MAX or
INT_MIN.)

@comment The bugs might not be worth noting, given the mass non-C99/POSIX
@comment behavior of much of the Newlib math library.
@comment BUGS
@comment On errors, errno is not set per C99 and POSIX requirements even if
@comment (math_errhandling & MATH_ERRNO) is non-zero.

PORTABILITY
C99, POSIX
*/

/* ilogb(double x)
 * return the binary exponent of non-zero x
 * ilogb(0) = 0x80000001
 * ilogb(inf/NaN) = 0x7fffffff (no signal is raised)
 */

#include <limits.h>
#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	int ilogb(double x)
#else
	int ilogb(x)
	double x;
#endif
{
	__int32_t hx,lx,ix;

	EXTRACT_WORDS(hx,lx,x);
	hx &= 0x7fffffff;
	if(hx<0x00100000) {
	    if((hx|lx)==0) 
		return FP_ILOGB0;	/* ilogb(0) = special case error */
	    else			/* subnormal x */
		if(hx==0) {
		    for (ix = -1043; lx>0; lx<<=1) ix -=1;
		} else {
		    for (ix = -1022,hx<<=11; hx>0; hx<<=1) ix -=1;
		}
	    return ix;
	}
	else if (hx<0x7ff00000) return (hx>>20)-1023;
	#if FP_ILOGBNAN != INT_MAX
	else if (hx>0x7ff00000) return FP_ILOGBNAN;	/* NAN */
	#endif
	else return INT_MAX;	/* infinite (or, possibly, NAN) */
}

#endif /* _DOUBLE_IS_32BITS */
