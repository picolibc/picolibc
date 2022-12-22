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
<<trunc>>, <<truncf>>---round to integer, towards zero
INDEX
	trunc
INDEX
	truncf

SYNOPSIS
	#include <math.h>
	double trunc(double <[x]>);
	float truncf(float <[x]>);

DESCRIPTION
	The <<trunc>> functions round their argument to the integer value, in
	floating format, nearest to but no larger in magnitude than the
	argument, regardless of the current rounding direction.  (While the
	"inexact" floating-point exception behavior is unspecified by the C
	standard, the <<trunc>> functions are written so that "inexact" is not
	raised if the result does not equal the argument, which behavior is as
	recommended by IEEE 754 for its related functions.)

RETURNS
<[x]> truncated to an integral value.

PORTABILITY
ANSI C, POSIX

*/

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

__float64
trunc64(__float64 x)
{
    int64_t ix = _asint64(x);
    int64_t mask;
    int exp;

    /* Un-biased exponent */
    exp = _exponent64(ix) - 1023;

    /* Inf/NaN, evaluate value */
    if (unlikely(exp == 1024))
        return x + x;

    /* compute portion of value with useful bits */
    if (exp < 0)
        /* less than one, save sign bit */
        mask = 0x8000000000000000LL;
    else
        /* otherwise, save sign, exponent and any useful bits */
        mask = ~(0x000fffffffffffffLL >> exp);

    return _asdouble(ix & mask);
}

_MATH_ALIAS_d_d(trunc)

#endif /* _NEED_FLOAT64 */
