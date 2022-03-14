/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
/*
FUNCTION
<<fmax>>, <<fmaxf>>---maximum
INDEX
	fmax
INDEX
	fmaxf

SYNOPSIS
	#include <math.h>
	double fmax(double <[x]>, double <[y]>);
	float fmaxf(float <[x]>, float <[y]>);

DESCRIPTION
The <<fmax>> functions determine the maximum numeric value of their arguments.
NaN arguments are treated as missing data:  if one argument is a NaN and the
other numeric, then the <<fmax>> functions choose the numeric value.

RETURNS
The <<fmax>> functions return the maximum numeric value of their arguments.

PORTABILITY
ANSI C, POSIX.

*/

#include "fdlibm.h"

#ifndef _DOUBLE_IS_32BITS

double fmax(double x, double y)
{
    if (issignaling(x) || issignaling(y))
        return x + y;

    if (isnan(x))
        return y;

    if (isnan(y))
        return x;

    return x > y ? x : y;
}

#endif /* _DOUBLE_IS_32BITS */
