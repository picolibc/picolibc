/*
Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.

Permission to use, copy, modify, and distribute this software
is freely granted, provided that this notice is preserved.
 */
/*
FUNCTION
<<fma>>, <<fmaf>>---floating multiply add
INDEX
	fma
INDEX
	fmaf

SYNOPSIS
	#include <math.h>
	double fma(double <[x]>, double <[y]>, double <[z]>);
	float fmaf(float <[x]>, float <[y]>, float <[z]>);

DESCRIPTION
The <<fma>> functions compute (<[x]> * <[y]>) + <[z]>, rounded as one ternary
operation:  they compute the value (as if) to infinite precision and round once
to the result format, according to the rounding mode characterized by the value
of FLT_ROUNDS.  That is, they are supposed to do this:  see below.

RETURNS
The <<fma>> functions return (<[x]> * <[y]>) + <[z]>, rounded as one ternary
operation.

BUGS
This implementation does not provide the function that it should, purely
returning "(<[x]> * <[y]>) + <[z]>;" with no attempt at all to provide the
simulated infinite precision intermediates which are required.  DO NOT USE THEM.

If double has enough more precision than float, then <<fmaf>> should provide
the expected numeric results, as it does use double for the calculation.  But
since this is not the case for all platforms, this manual cannot determine
if it is so for your case.

PORTABILITY
ANSI C, POSIX.

*/

#include "fdlibm.h"

#if !_HAVE_FAST_FMA

#ifdef _NEED_FLOAT64

__float64
fma64(__float64 x, __float64 y, __float64 z)
{
  /* Implementation defined. */
  return (x * y) + z;
}

_MATH_ALIAS_d_ddd(fma)

#endif /* _NEED_FLOAT64 */

#endif /* !_HAVE_FAST_FMA */
