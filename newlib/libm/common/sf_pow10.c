/* sf_pow10.c -- float version of s_pow10.c.
 * Modification of sf_pow10.c by Yaakov Selkowitz 2007.
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
 * wrapper pow10f(x)
 */

#undef pow10f
#include "fdlibm.h"
#include <errno.h>
#include <math.h>

float
pow10f(float x)		/* wrapper pow10f */
{
  return _powf(10.0, x);
}

#undef pow10
#undef pow10l

_MATH_ALIAS_f_f(pow10)
