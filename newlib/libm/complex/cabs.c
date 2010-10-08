/* $NetBSD: cabs.c,v 1.1 2007/08/20 16:01:30 drochner Exp $ */

/*
 * Written by Matthias Drochner <drochner@NetBSD.org>.
 * Public domain.
 *
 * imported and modified include for newlib 2010/10/03 
 * Marco Atzeri <marco_atzeri@yahoo.it>
 */

#include <complex.h>
#include <math.h>

double
cabs(double complex z)
{

	return hypot( creal(z), cimag(z) );
}
