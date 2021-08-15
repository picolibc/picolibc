/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
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

#define _DEFAULT_SOURCE
#include <fenv.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __STDC_IEC_559__
#define HAVE_HW_DOUBLE
#endif

#define scat(a,b) a ## b

/* Tests with doubles */
#ifdef HAVE_HW_DOUBLE
#define EXCEPTION_TEST	MATH_ERREXCEPT
#else
#define EXCEPTION_TEST	0
#endif

#define BIG 1.7e308
#define SMALL 5e-324
#define FLOAT_T double

#define makemathname(s) s
#define makemathname_r(s) scat(s,_r)

#include "math_errhandling_tests.c"

#undef BIG
#undef SMALL
#undef makemathname
#undef makemathname_r
#undef FLOAT_T
#undef EXCEPTION_TEST

/* Tests with floats */
#define EXCEPTION_TEST	MATH_ERREXCEPT
#define BIG 3e38
#define SMALL 1e-45
#define FLOAT_T float
#define makemathname(s) scat(s,f)
#define makemathname_r(s) scat(s,f_r)

#include "math_errhandling_tests.c"

int main()
{
	int result = 0;

#ifdef HAVE_HW_DOUBLE
	printf("Double tests:\n");
	result += run_tests();
#endif
	printf("Float tests:\n");
	result += run_testsf();
	return result;
}
