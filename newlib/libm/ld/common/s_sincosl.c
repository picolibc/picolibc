/* s_sincosl.c -- long double version of s_sincos.c
 *
 * Copyright (C) 2013 Elliot Saba
 * Developed at the University of Washington
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
*/

void
sincosl( long double x, long double * s, long double * c )
{
    *s = _sinl( x );
    *c = _cosl( x );
}

#if __LDBL_MANT_DIG__ == 113
#if defined(_HAVE_ALIAS_ATTRIBUTE)
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
__strong_reference(sincosl, __sincosieee128);
#endif
#endif
