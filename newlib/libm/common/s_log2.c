/* @(#)s_log2.c 5.1 93/09/24 */
/* Modification from s_exp10.c Yaakov Selkowitz 2009.  */

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
	<<log2>>, <<log2f>>---base 2 logarithm
INDEX
	log2
INDEX
	log2f

SYNOPSIS
	#include <math.h>
	double log2(double <[x]>);
	float log2f(float <[x]>);

DESCRIPTION
The <<log2>> functions compute the base-2 logarithm of <[x]>.  A domain error
occurs if the argument is less than zero.  A range error occurs if the
argument is zero.

The Newlib implementations are not full, intrinisic calculations, but
rather are derivatives based on <<log>>.  (Accuracy might be slightly off from
a direct calculation.)  In addition to functions, they are also implemented as
macros defined in math.h:
. #define log2(x) (log (x) / _M_LN2)
. #define log2f(x) (logf (x) / (float) _M_LN2)
To use the functions instead, just undefine the macros first.

RETURNS
The <<log2>> functions return
@ifnottex
<<log base-2(<[x]>)>>
@end ifnottex
@tex
$log_2(x)$
@end tex
on success.
When <[x]> is zero, the
returned value is <<-HUGE_VAL>> and <<errno>> is set to <<ERANGE>>.
When <[x]> is negative, the returned value is NaN (not a number) and
<<errno>> is set to <<EDOM>>.

PORTABILITY
C99, POSIX, System V Interface Definition (Issue 6).
*/

/*
 * wrapper log2(x)
 */

#include "fdlibm.h"
#if __OBSOLETE_MATH_DOUBLE
#include <errno.h>
#include <math.h>
#undef log2

#ifdef _NEED_FLOAT64

__float64
log264(__float64 x)		/* wrapper log2 */
{
    return (log64(x) / (__float64) _M_LN2_LD);
}

_MATH_ALIAS_d_d(log2)

#endif /* _NEED_FLOAT64 */
#else
#include "log2.c"
#endif /* __OBSOLETE_MATH_DOUBLE */
