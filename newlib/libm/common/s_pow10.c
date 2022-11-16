/* @(#)s_pow10.c 5.1 93/09/24 */
/* Modification from s_exp10.c Yaakov Selkowitz 2007.  */

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
	<<pow10>>, <<pow10f>>---base 10 power functions
INDEX
	pow10
INDEX
	pow10f

SYNOPSIS
	#include <math.h>
	double pow10(double <[x]>);
	float pow10f(float <[x]>);

DESCRIPTION
	<<pow10>> and <<pow10f>> calculate 10 ^ <[x]>, that is, 
	@ifnottex
	10 raised to the power <[x]>.
	@end ifnottex
	@tex
	$10^x$
	@end tex

RETURNS
	On success, <<pow10>> and <<pow10f>> return the calculated value.
	If the result underflows, the returned value is <<0>>.  If the
	result overflows, the returned value is <<HUGE_VAL>>.  In
	either case, <<errno>> is set to <<ERANGE>>.

PORTABILITY
	<<pow10>> and <<pow10f>> are GNU extensions.
*/

/*
 * wrapper pow10(x)
 */

#undef pow10
#include "fdlibm.h"
#include <errno.h>
#include <math.h>

#ifdef _NEED_FLOAT64

__float64 pow1064(__float64 x)		/* wrapper pow10 */
{
  return _pow64(_F_64(10.0), x);
}

_MATH_ALIAS_d_d(pow10)

#endif /* _NEED_FLOAT64 */
