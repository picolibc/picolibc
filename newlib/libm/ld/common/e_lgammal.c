/* @(#)e_lgamma.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

long double
lgammal(long double x)
{
	return (lgammal_r(x, &signgam));
}

#  ifdef _HAVE_ALIAS_ATTRIBUTE
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wattribute-alias="
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(lgammal, gammal);
#else
long double
gammal(long double x)
{
    return lgammal(x);
}
#endif
