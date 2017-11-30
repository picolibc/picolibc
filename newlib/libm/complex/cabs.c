/* $NetBSD: cabs.c,v 1.1 2007/08/20 16:01:30 drochner Exp $ */

/*
 * Written by Matthias Drochner <drochner@NetBSD.org>.
 * Public domain.
 *
 * imported and modified include for newlib 2010/10/03 
 * Marco Atzeri <marco_atzeri@yahoo.it>
 */

/*
FUNCTION
        <<cabs>>, <<cabsf>>, <<cabsl>>---complex absolute-value

INDEX
        cabs
INDEX
        cabsf
INDEX
        cabsl

SYNOPSIS
       #include <complex.h>
       double cabs(double complex <[z]>);
       float cabsf(float complex <[z]>);
       long double cabsl(long double complex <[z]>);


DESCRIPTION
        These functions compute compute the complex absolute value 
        (also called norm, modulus, or magnitude) of <[z]>. 

        <<cabsf>> is identical to <<cabs>>, except that it performs
        its calculations on <<float complex>>.

        <<cabsl>> is identical to <<cabs>>, except that it performs
        its calculations on <<long double complex>>.

RETURNS
        The cabs* functions return the complex absolute value.

PORTABILITY
        <<cabs>>, <<cabsf>> and <<cabsl>> are ISO C99

QUICKREF
        <<cabs>>, <<cabsf>> and <<cabsl>> are ISO C99

*/


#include <complex.h>
#include <math.h>

double
cabs(double complex z)
{

	return hypot( creal(z), cimag(z) );
}
