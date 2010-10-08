/* $NetBSD: cimagf.c,v 1.2 2010/09/15 16:11:29 christos Exp $ */

/*
 * Written by Matthias Drochner <drochner@NetBSD.org>.
 * Public domain.
 *
 * imported and modified include for newlib 2010/10/03 
 * Marco Atzeri <marco_atzeri@yahoo.it>
 */

#include <complex.h>
#include "../common/fdlibm.h"

float
cimagf(float complex z)
{
	float_complex w = { .z = z };

	return (IMAG_PART(w));
}
