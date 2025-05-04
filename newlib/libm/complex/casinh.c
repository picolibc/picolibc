/* $NetBSD: casinh.c,v 1.1 2007/08/20 16:01:31 drochner Exp $ */

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
        <<casinh>>, <<casinhf>>---complex arc hyperbolic sine

INDEX
        casinh
INDEX
        casinhf

SYNOPSIS
       #include <complex.h>
       double complex casinh(double complex <[z]>);
       float complex casinhf(float complex <[z]>);


DESCRIPTION
        @ifnottex
        These functions compute the complex arc hyperbolic sine of <[z]>,
        with branch cuts outside the interval [-i, +i] along the 
        imaginary axis.        
        @end ifnottex
        @tex
        These functions compute the complex arc hyperbolic sine of <[z]>,
        with branch cuts outside the interval [$-i$, $+i$] along the 
        imaginary axis.        
        @end tex

        <<casinhf>> is identical to <<casinh>>, except that it performs
        its calculations on <<floats complex>>.

RETURNS
        @ifnottex
        These functions return the complex arc hyperbolic sine value, 
        in the range of a strip mathematically unbounded along the 
        real axis and in the interval [-i*p/2, +i*p/2] along the 
        imaginary axis.
        @end ifnottex
        @tex
        These functions return the complex arc hyperbolic sine value, 
        in the range of a strip mathematically unbounded along the 
        real axis and in the interval [$-i\pi/2$, $+i\pi/2$] along the 
        imaginary axis.
        @end tex

PORTABILITY
        <<casinh>> and <<casinhf>> are ISO C99

QUICKREF
        <<casinh>> and <<casinhf>> are ISO C99

*/


#include <complex.h>
#include <math.h>
#include <float.h>

double complex
casinh(double complex z)
{

    double x = fabs(creal(z));
    double y = fabs(cimag(z));
    double complex res;
    double complex w;

    const double eps = DBL_EPSILON;

    if (y == 0.0) {
        if (isnan(x)) {
            res = NAN + copysign(0.0, cimag(z)) * I;
        }
        else if (isinf(x)) {
            res = x + copysign(0.0, cimag(z)) * I;
        }
        else {
            res = asinh(x) + copysign(0.0, cimag(z)) * I;
        }
    }
    /* Handle large values */
    else if (x >= 1.0/eps || y >= 1.0/eps) {
        res = clog(x + y * I);
        res = creal(res) + M_LN2 + cimag(res) * I;

    }

    /* Case where real part >= 0.5 and imag part very samll */
    else if (x >= 0.5 && y < eps/8.0) {
        double s = hypot(1.0, x);
        res = log(x + s) + atan2(y, s) * I;
    }

    /* Case Where real part very small and imag part >= 1.5 */
    else if (x < eps/8.0 && y >= 1.5) {
        double s = sqrt((y + 1.0) * (y - 1.0));
        res = log(y + s) + atan2(s, x) * I;
    }

    else {
        /* General case */
        w = (x - y) * (x + y) + 1.0 + (2.0 * x * y) * I;
        w = csqrt(w);

        w = (x + creal(w)) + (y + cimag(w)) * I;
        res = clog(w);
    }

    /* Apply correct signs */
    res = copysign(creal(res), creal(z)) +
          copysign(cimag(res), cimag(z)) * I;

    return res;
}
