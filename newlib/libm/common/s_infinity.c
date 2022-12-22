/*
Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.

Developed at SunPro, a Sun Microsystems, Inc. business.
Permission to use, copy, modify, and distribute this
software is freely granted, provided that this notice 
is preserved.
 */
/*
 * infinity () returns the representation of infinity.
 * Added by Cygnus Support.
 */

/*
FUNCTION
	<<infinity>>, <<infinityf>>---representation of infinity

INDEX
	infinity
INDEX
	infinityf

SYNOPSIS
	#include <math.h>
	double infinity(void);
	float infinityf(void);

DESCRIPTION
	<<infinity>> and <<infinityf>> return the special number IEEE
	infinity in double- and single-precision arithmetic
	respectively.

PORTABILITY
<<infinity>> and <<infinityf>> are neither standard C nor POSIX.  C and
POSIX require macros HUGE_VAL and HUGE_VALF to be defined in math.h, which
Newlib defines to be infinities corresponding to these archaic infinity()
and infinityf() functions in floating-point implementations which do have
infinities.

QUICKREF
	infinity - pure

*/

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

__float64
infinity64(void)
{
	__float64 x;

	INSERT_WORDS(x,0x7ff00000,0);
	return x;
}

_MATH_ALIAS_d(infinity)

#endif /* _NEED_FLOAT64 */
