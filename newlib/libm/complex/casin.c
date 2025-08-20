/* $NetBSD: casin.c,v 1.1 2007/08/20 16:01:31 drochner Exp $ */

/*-
 * Copyright (c) 2007 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software written by Stephen L. Moshier.
 * It is redistributed by the NetBSD Foundation by permission of the author.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * imported and modified include for newlib 2010/10/03 
 * Marco Atzeri <marco_atzeri@yahoo.it>
 */

/*
FUNCTION
        <<casin>>, <<casinf>>---complex arc sine

INDEX
        casin
INDEX
        casinf

SYNOPSIS
       #include <complex.h>
       double complex casin(double complex <[z]>);
       float complex casinf(float complex <[z]>);


DESCRIPTION
        These functions compute the complex arc sine of <[z]>,
        with branch cuts outside the interval [-1, +1] along the real axis.

        <<casinf>> is identical to <<casin>>, except that it performs
        its calculations on <<floats complex>>.

RETURNS
        @ifnottex
        These functions return the complex arc sine value, in the range
        of a strip mathematically  unbounded  along the imaginary axis
        and in the interval [-pi/2, +pi/2] along the real axis.
        @end ifnottex
        @tex
        These functions return the complex arc sine value, in the range
        of a strip mathematically  unbounded  along the imaginary axis
        and in the interval [$-\pi/2$, $+\pi/2$] along the real axis.
        @end tex

PORTABILITY
        <<casin>> and <<casinf>> are ISO C99

QUICKREF
        <<casin>> and <<casinf>> are ISO C99

*/


#include <complex.h>
#include <math.h>

#ifdef __weak_alias
__weak_alias(casin, _casin)
#endif

double complex
casin(double complex z)
{
	double x = creal(z);
	double y = cimag(z);
	double complex res;

	if (x == 0.0 && y == 0.0) return z;

        if (isnan(x) || isnan(y)) {
                if (isinf(x) || isinf(y)) {
                        return CMPLX(NAN, copysign((double) INFINITY, y));
                }
                return CMPLX((double) NAN, (double) NAN);
        }

	double complex iz = CMPLX(-y,x);
	double complex w = casinh(iz);
	res = CMPLX(cimag(w), - creal(w));

	return res;
}
