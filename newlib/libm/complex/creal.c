/* $NetBSD: creal.c,v 1.2 2010/09/15 16:11:29 christos Exp $ */

/*
 * Written by Matthias Drochner <drochner@NetBSD.org>.
 * Public domain.
 *
 * imported and modified include for newlib 2010/10/03 
 * Marco Atzeri <marco_atzeri@yahoo.it>
 */

/*
FUNCTION
        <<creal>>, <<crealf>>, <<creall>>---real part

INDEX
        creal
INDEX
        crealf
INDEX
        creall

ANSI_SYNOPSIS
       #include <complex.h>
       double creal(double complex <[z]>);
       float crealf(float complex <[z]>);
       double long creall(long double complex <[z]>);

       
DESCRIPTION
        These functions compute the real part of <[z]>.

        <<crealf>> is identical to <<creal>>, except that it performs
        its calculations on <<float complex>>.

        <<creall>> is identical to <<creal>>, except that it performs
        its calculations on <<long double complex>>.

RETURNS
        The creal* functions return the real part value.

PORTABILITY
        <<creal>>, <<crealf>> and <<creall>> are ISO C99

QUICKREF
        <<creal>>, <<crealf>> and <<creall>> are ISO C99

*/


#include <complex.h>
#include "../common/fdlibm.h"

double
creal(double complex z)
{
	double_complex w = { .z = z };

	return (REAL_PART(w));
}
