/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */
/*
FUNCTION
<<fdim>>, <<fdimf>>---positive difference
INDEX
	fdim
INDEX
	fdimf

SYNOPSIS
	#include <math.h>
	double fdim(double <[x]>, double <[y]>);
	float fdimf(float <[x]>, float <[y]>);

DESCRIPTION
The <<fdim>> functions determine the positive difference between their
arguments, returning:
.	<[x]> - <[y]>	if <[x]> > <[y]>, or
	@ifnottex
.	+0	if <[x]> <= <[y]>, or
	@end ifnottex
	@tex
.	+0	if <[x]> $\leq$ <[y]>, or
	@end tex
.	NAN	if either argument is NAN.
A range error may occur.

RETURNS
The <<fdim>> functions return the positive difference value.

PORTABILITY
ANSI C, POSIX.

*/

#include "fdlibm.h"

#ifdef _NEED_FLOAT64

__float64
fdim64(__float64 x, __float64 y)
{
  if (isnan(x) || isnan(y)) return(x+y);

  __float64 z = x > y ? x - y : _F_64(0.0);
  if (!isinf(x) && !isinf(y))
    z = check_oflow(z);
  return z;
}

_MATH_ALIAS_d_dd(fdim)

#endif /* _NEED_FLOAT64 */
