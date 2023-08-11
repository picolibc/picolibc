/* Copyright (c) 2016 Yaakov Selkowitz <yselkowi@redhat.com> */
/*
FUNCTION
        <<clog10>>, <<clog10f>>---complex base-10 logarithm

INDEX
        clog10
INDEX
        clog10f

SYNOPSIS
       #define _DEFAULT_SOURCE
       #include <complex.h>
       double complex clog10(double complex <[z]>);
       float complex clog10f(float complex <[z]>);


DESCRIPTION
        These functions compute the complex base-10 logarithm of <[z]>.
        <<clog10>> is equivalent to <<clog>>(<[z]>)/<<log>>(10).

        <<clog10f>> is identical to <<clog10>>, except that it performs
        its calculations on <<floats complex>>.

RETURNS
        The clog10 functions return the complex base-10 logarithm value.

PORTABILITY
        <<clog10>> and <<clog10f>> are GNU extensions.

*/

#define _DEFAULT_SOURCE
#include <complex.h>
#include <math.h>

double complex
clog10(double complex z)
{
	double p, rr;

	rr = cabs(z);
	p = log10(rr);
	rr = atan2(cimag(z), creal(z)) * M_IVLN10;
	return (double complex) p + rr * (double complex) I;
}
