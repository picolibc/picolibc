/* $NetBSD: cargl.c,v 1.1 2014/10/10 00:48:18 christos Exp $ */

/*
 * Public domain.
 */

#include <complex.h>
#include <math.h>

/* On platforms where long double is as wide as double.  */
#ifdef _LDBL_EQ_DBL
long double
cargl(long double complex z)
{
         return carg (z);
}
#endif
