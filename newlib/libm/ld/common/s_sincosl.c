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
