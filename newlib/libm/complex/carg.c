/*
Copyright (c) 2007 The NetBSD Foundation, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */
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
