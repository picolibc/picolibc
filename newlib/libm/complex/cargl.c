/* $NetBSD: cargl.c,v 1.1 2014/10/10 00:48:18 christos Exp $ */

/*
 * Public domain.
 */

#include <complex.h>
#include <math.h>

long double
cargl(long double complex z)
{     
       #ifdef _LDBL_EQ_DBL
         return carg (z);
       #else
         return atan2l (cimagl (z), creall (z));
       #endif
}
