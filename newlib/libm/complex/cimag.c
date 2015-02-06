/* $NetBSD: cimag.c,v 1.2 2010/09/15 16:11:29 christos Exp $ */

/*
 * Written by Matthias Drochner <drochner@NetBSD.org>.
 * Public domain.
 *
 * imported and modified include for newlib 2010/10/03 
 * Marco Atzeri <marco_atzeri@yahoo.it>
 */

/*
FUNCTION
        <<cimag>>, <<cimagf>>, <<cimagl>>---imaginary part

INDEX
        cimag
INDEX
        cimagf
INDEX
        cimagl

ANSI_SYNOPSIS
       #include <complex.h>
       double cimag(double complex <[z]>);
       float cimagf(float complex <[z]>);
       long double cimagl(long double complex <[z]>);


DESCRIPTION
        These functions compute the imaginary part of <[z]>.

        <<cimagf>> is identical to <<cimag>>, except that it performs
        its calculations on <<float complex>>.

        <<cimagl>> is identical to <<cimag>>, except that it performs
        its calculations on <<long double complex>>.

RETURNS
        The cimag* functions return the imaginary part value (as a real).

PORTABILITY
        <<cimag>>, <<cimagf>> and <<cimagl>> are ISO C99

QUICKREF
        <<cimag>>, <<cimagf>> and <<cimagl>> are ISO C99

*/


#include <complex.h>
#include "../common/fdlibm.h"

double
cimag(double complex z)
{
	double_complex w = { .z = z };

	return (IMAG_PART(w));
}
