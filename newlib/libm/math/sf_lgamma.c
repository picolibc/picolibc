/* wf_lgamma.c -- float version of w_lgamma.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
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
 *
 */

#include "fdlibm.h"
#include <errno.h>

float
lgammaf(float x)
{
    return lgammaf_r(x, &__signgam);
}

#  ifdef _HAVE_ALIAS_ATTRIBUTE
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wattribute-alias="
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(lgammaf, gammaf);
#else
float
gammaf(float x)
{
    return lgammaf(x);
}
#endif

_MATH_ALIAS_f_f(lgamma)
_MATH_ALIAS_f_f(gamma)
