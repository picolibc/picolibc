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

#if __FLT_EVAL_METHOD__ == 2 && defined(_HAVE_LONG_DOUBLE)

__float64
fma64(__float64 x, __float64 y, __float64 z)
{
    return (__float64) fmal((long double) x, (long double) y, (long double) z);
}

#else

typedef __float64 FLOAT_T;

#define FMA fma64
#define NEXTAFTER nextafter64
#define LDEXP ldexp64
#define FREXP frexp64
#define SCALBN scalbn64
#define ILOGB    ilogb64
#define COPYSIGN copysign64

#define SPLIT ((FLOAT_T) 0x1p26 + (FLOAT_T) 1.0)
#define FLOAT_MANT_DIG        _FLOAT64_MANT_DIG
#define FLOAT_MAX_EXP         _FLOAT64_MAX_EXP
#define FLOAT_MIN             _FLOAT64_MIN

static inline int
odd_mant(FLOAT_T x)
{
    return asuint64(x) & 1;
}

static unsigned int
EXPONENT(FLOAT_T x)
{
    return _exponent64(asuint64(x));
}

#ifdef PICOLIBC_FLOAT64_NOEXCEPT
#define feraiseexcept(x) ((void) (x))
#endif

#include "fma_inc.h"

#endif

_MATH_ALIAS_d_ddd(fma)

#endif /* _NEED_FLOAT64 */

#endif /* !_HAVE_FAST_FMA */
