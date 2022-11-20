
/* @(#)w_exp2.c 5.1 93/09/24 */
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
	<<exp2>>, <<exp2f>>---exponential, base 2
INDEX
	exp2
INDEX
	exp2f

SYNOPSIS
	#include <math.h>
	double exp2(double <[x]>);
	float exp2f(float <[x]>);

DESCRIPTION
	<<exp2>> and <<exp2f>> calculate 2 ^ <[x]>, that is,
	@ifnottex
	2 raised to the power <[x]>.
	@end ifnottex
	@tex
	$2^x$
	@end tex

RETURNS
	On success, <<exp2>> and <<exp2f>> return the calculated value.
	If the result underflows, the returned value is <<0>>.  If the
	result overflows, the returned value is <<HUGE_VAL>>.  In
	either case, <<errno>> is set to <<ERANGE>>.

PORTABILITY
	ANSI C, POSIX.

*/

/*
 * wrapper exp2(x)
 */

#include "fdlibm.h"
#if __OBSOLETE_MATH_DOUBLE
#ifdef _NEED_FLOAT64

extern __float64 _pow64(__float64, __float64);

__float64 __inhibit_new_builtin_calls
exp264(__float64 x) /* wrapper exp2 */
{
    return _pow64(2.0, x);
}

_MATH_ALIAS_d_d(exp2)

#endif /* _NEED_FLOAT64 */
#else
#include "../common/exp2.c"
#endif /* __OBSOLETE_MATH_DOUBLE */
