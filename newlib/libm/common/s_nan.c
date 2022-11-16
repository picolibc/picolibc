/*
Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.

Developed at SunPro, a Sun Microsystems, Inc. business.
Permission to use, copy, modify, and distribute this
software is freely granted, provided that this notice 
is preserved.
 */
/*
 * nan () returns a nan.
 * Added by Cygnus Support.
 */

/*
FUNCTION
	<<nan>>, <<nanf>>---representation of ``Not a Number''

INDEX
	nan
INDEX
	nanf

SYNOPSIS
	#include <math.h>
	double nan(const char *<[unused]>);
	float nanf(const char *<[unused]>);


DESCRIPTION
	<<nan>> and <<nanf>> return an IEEE NaN (Not a Number) in
	double- and single-precision arithmetic respectively.  The
	argument is currently disregarded.

QUICKREF
	nan - pure

*/

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

__float64
nan64(const char *unused)
{
	__float64 x;

        (void) unused;
#if __GNUC_PREREQ (3, 3)
	x = __builtin_nan("");
#else
	INSERT_WORDS(x,0x7ff80000,0);
#endif
	return x;
}

_MATH_ALIAS_d_s(nan)

#endif /* _NEED_FLOAT64 */
