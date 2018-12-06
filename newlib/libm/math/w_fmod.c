
/* @(#)w_fmod.c 5.1 93/09/24 */
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
<<fmod>>, <<fmodf>>---floating-point remainder (modulo)

INDEX
fmod
INDEX
fmodf

SYNOPSIS
#include <math.h>
double fmod(double <[x]>, double <[y]>);
float fmodf(float <[x]>, float <[y]>);

DESCRIPTION
The <<fmod>> and <<fmodf>> functions compute the floating-point
remainder of <[x]>/<[y]> (<[x]> modulo <[y]>).

RETURNS
The <<fmod>> function returns the value 
@ifnottex
<[x]>-<[i]>*<[y]>, 
@end ifnottex
@tex
$x-i\times y$,
@end tex
for the largest integer <[i]> such that, if <[y]> is nonzero, the
result has the same sign as <[x]> and magnitude less than the
magnitude of <[y]>. 

<<fmod(<[x]>,0)>> returns NaN, and sets <<errno>> to <<EDOM>>.

PORTABILITY
<<fmod>> is ANSI C. <<fmodf>> is an extension.
*/

/* 
 * wrapper fmod(x,y)
 */

#include "fdlibm.h"
#include <errno.h>

#ifndef _DOUBLE_IS_32BITS

#ifdef __STDC__
	double fmod(double x, double y)	/* wrapper fmod */
#else
	double fmod(x,y)		/* wrapper fmod */
	double x,y;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_fmod(x,y);
#else
	double z;
	z = __ieee754_fmod(x,y);
	if(_LIB_VERSION == _IEEE_ ||isnan(y)||isnan(x)) return z;
	if(y==0.0) {
	    /* fmod(x,0) */
	    errno = EDOM;
	    return 0.0/0.0;
	} else
	    return z;
#endif
}

#endif /* defined(_DOUBLE_IS_32BITS) */
