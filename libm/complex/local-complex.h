/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LOCAL_COMPLEX_H_
#define _LOCAL_COMPLEX_H_

#define _DEFAULT_SOURCE

#include <complex.h>
#include <math.h>

#ifdef __GNUCLIKE_PRAGMA_DIAGNOSTIC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
/* GCC analyzer gets confused about the use of 'CMPLXL' here */
#pragma GCC diagnostic ignored "-Wanalyzer-use-of-uninitialized-value"
#endif

void  _cchshf(float, float *, float *);
float _redupif(float);
float _ctansf(float complex);

#define M_PIF   ((float)M_PI)
#define M_PI_2F ((float)M_PI_2)

void        _cchsh(double, double *, double *);
double      _redupi(double);
double      _ctans(double complex);

void        _cchshl(long double, long double *, long double *);
long double _redupil(long double);
long double _ctansl(long double complex);

#define M_PIL   3.14159265358979323846264338327950280e+00L
#define M_PI_2L 1.57079632679489661923132169163975140e+00L

#endif /* _LOCAL_COMPLEX_H_ */
