/* $NetBSD: carg.c,v 1.1 2007/08/20 16:01:31 drochner Exp $ */

/*
 * Written by Matthias Drochner <drochner@NetBSD.org>.
 * Public domain.
 *
 * imported and modified include for newlib 2010/10/03 
 * Marco Atzeri <marco_atzeri@yahoo.it>
 */

/*
FUNCTION
        <<carg>>, <<cargf>>---argument (phase angle)

INDEX
        carg
INDEX
        cargf

SYNOPSIS
       #include <complex.h>
       double carg(double complex <[z]>);
       float cargf(float complex <[z]>);


DESCRIPTION
        These functions compute the argument (also called phase angle) 
        of <[z]>, with a branch cut along the negative real axis.

        <<cargf>> is identical to <<carg>>, except that it performs
        its calculations on <<floats complex>>.

RETURNS
        @ifnottex
        The carg functions return the value of the argument in the 
        interval [-pi, +pi]
        @end ifnottex
        @tex
        The carg functions return the value of the argument in the 
        interval [$-\pi$, $+\pi$]
        @end tex

PORTABILITY
        <<carg>> and <<cargf>> are ISO C99

QUICKREF
        <<carg>> and <<cargf>> are ISO C99

*/

#include <complex.h>
#include <math.h>

double
carg(double complex z)
{

	return atan2( cimag(z) , creal(z) );
}
