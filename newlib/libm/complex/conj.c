/* $NetBSD: conj.c,v 1.2 2010/09/15 16:11:29 christos Exp $ */

/*
 * Written by Matthias Drochner <drochner@NetBSD.org>.
 * Public domain.
 *
 * imported and modified include for newlib 2010/10/03 
 * Marco Atzeri <marco_atzeri@yahoo.it>
 */

/*
FUNCTION
        <<conj>>, <<conjf>>---complex conjugate

INDEX
        conj
INDEX
        conjf

ANSI_SYNOPSIS
       #include <complex.h>
       double complex conj(double complex <[z]>);
       float complex conjf(float complex <[z]>);


DESCRIPTION
        These functions compute the complex conjugate of <[z]>, 
        by reversing the sign of its imaginary part.

        <<conjf>> is identical to <<conj>>, except that it performs
        its calculations on <<floats complex>>.

RETURNS
        The conj functions return the complex conjugate value.

PORTABILITY
        <<conj>> and <<conjf>> are ISO C99

QUICKREF
        <<conj>> and <<conjf>> are ISO C99

*/

#include <complex.h>
#include "../common/fdlibm.h"

double complex
conj(double complex z)
{
	double_complex w = { .z = z };

	IMAG_PART(w) = -IMAG_PART(w);

	return (w.z);
}
