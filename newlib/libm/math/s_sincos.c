/* Copyright (c) 2002 Thomas Fitzsimmons <fitzsim@redhat.com> */
/* sincos -- currently no more efficient than two separate calls to
   sin and cos. */

#include "fdlibm.h"
#include <errno.h>
#include <math.h>

#ifndef _DOUBLE_IS_32BITS

#ifdef _HAVE_ALIAS_ATTRIBUTE
extern double _sin(double);
extern double _cos(double);
#else
#define _sin sin
#define _cos cos
#endif

void __inhibit_new_builtin_calls
sincos(double x, double *sinx, double *cosx)
{
    *sinx = _sin(x);
    *cosx = _cos(x);
}

#endif /* defined(_DOUBLE_IS_32BITS) */
