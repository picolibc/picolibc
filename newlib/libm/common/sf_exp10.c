/* sf_exp10.c -- float version of s_exp10.c.
 * Modification of sf_exp2.c by Yaakov Selkowitz 2007.
 */

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
 * wrapper exp10f(x)
 */

#undef exp10f
#include "fdlibm.h"
#include <errno.h>
#include <math.h>

float exp10f(float x)		/* wrapper exp10f */
{
  return _powf(10.0, x);
}

_MATH_ALIAS_f_f(exp10)
