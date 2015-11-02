/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
/*
FUNCTION
<<signbit>>---Does floating-point number have negative sign?

INDEX
	signbit

ANSI_SYNOPSIS
	#include <math.h>
	int signbit(real-floating <[x]>);

DESCRIPTION
The <<signbit>> macro determines whether the sign of its argument value is
negative.  The macro reports the sign of all values, including infinities,
zeros, and NaNs.  If zero is unsigned, it is treated as positive.  As shown in
the synopsis, the argument is "real-floating," meaning that any of the real
floating-point types (float, double, etc.) may be given to it.

Note that because of the possibilities of signed 0 and NaNs, the expression
"<[x]> < 0.0" does not give the same result as <<signbit>> in all cases.

RETURNS
The <<signbit>> macro returns a nonzero value if and only if the sign of its
argument value is negative.

PORTABILITY
C99, POSIX.

*/

#include "fdlibm.h"

int __signbitf (float x);
int __signbitd (double x);

int
__signbitf (float x)
{
  __uint32_t w;

  GET_FLOAT_WORD(w,x);

  return (w & 0x80000000) != 0;
}

int
__signbitd (double x)
{
  __uint32_t msw;

  GET_HIGH_WORD(msw, x);

  return (msw & 0x80000000) != 0;
}
