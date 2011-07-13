/* $NetBSD: cacos.c,v 1.1 2007/08/20 16:01:30 drochner Exp $ */

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
        <<cacos>>, <<cacosf>>---complex arc cosine

INDEX
        cacos
INDEX
        cacosf

ANSI_SYNOPSIS
       #include <complex.h>
       double complex cacos(double complex <[z]>);
       float complex cacosf(float complex <[z]>);


DESCRIPTION
        These functions compute the complex arc cosine of <[z]>,
        with branch cuts outside the interval [-1, +1] along the real axis.

        <<cacosf>> is identical to <<cacos>>, except that it performs
        its calculations on <<floats complex>>.

RETURNS
        @ifnottex
        These functions return the complex arc cosine value, in the range
        of a strip mathematically  unbounded  along the imaginary axis
        and in the interval [0, pi] along the real axis.
        @end ifnottex
        @tex
        These functions return the complex arc cosine value, in the range
        of a strip mathematically  unbounded  along the imaginary axis
        and in the interval [<<0>>, $\pi$] along the real axis.
        @end tex

PORTABILITY
        <<cacos>> and <<cacosf>> are ISO C99

QUICKREF
        <<cacos>> and <<cacosf>> are ISO C99

*/

#include <complex.h>
#include <math.h>

double complex
cacos(double complex z)
{
	double complex w;

	/* FIXME: The original NetBSD code results in an ICE when trying to
	   build this function on ARM/Thumb using gcc 4.5.1.  For now we use
	   a hopefully temporary workaround. */
#if 0
	w = casin(z);
	w = (M_PI_2 - creal(w)) - cimag(w) * I;
#else
	double complex tmp0, tmp1;

	tmp0 = casin(z);
	tmp1 = M_PI_2 - creal(tmp0);
	w = tmp1 - (cimag(tmp0) * I);
#endif
	return w;
}
