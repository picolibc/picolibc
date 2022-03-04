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

#define _GNU_SOURCE
#include <fenv.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

static const char *
e_to_str(int e)
{
	if (e == 0)
		return "NONE";

#ifdef FE_DIVBYZERO
	if (e == FE_DIVBYZERO)
		return "FE_DIVBYZERO";
#endif
#ifdef FE_OVERFLOW
	if (e == FE_OVERFLOW)
		return "FE_OVERFLOW";
#endif
#ifdef FE_UNDERFLOW
	if (e == FE_UNDERFLOW)
		return "FE_UNDERFLOW";
#endif
#ifdef FE_INEXACT
	if (e == FE_INEXACT)
		return "FE_INEXACT";
#endif
#ifdef FE_INVALID
	if (e == FE_INVALID)
		return "FE_INVALID";
#endif
#if defined(FE_OVERFLOW) && defined(FE_INEXACT)
	if (e == (FE_OVERFLOW|FE_INEXACT))
		return "FE_OVERFLOW|FE_INEXACT";
#endif
#if defined(FE_UNDERFLOW) && defined(FE_INEXACT)
	if (e == (FE_UNDERFLOW|FE_INEXACT))
		return "FE_UNDERFLOW|FE_INEXACT";
#endif
	static char buf[3][128];
        static int i = 0;
        buf[i][0] = '\0';
        while (e) {
            char *v = NULL;
            char tmp[24];
#ifdef FE_DIVBYZERO
            if (e & FE_DIVBYZERO) {
		v = "FE_DIVBYZERO";
                e &= ~FE_DIVBYZERO;
            } else
#endif
#ifdef FE_OVERFLOW
            if (e & FE_OVERFLOW) {
		v = "FE_OVERFLOW";
                e &= ~FE_OVERFLOW;
            } else
#endif
#ifdef FE_UNDERFLOW
            if (e & FE_UNDERFLOW) {
		v = "FE_UNDERFLOW";
                e &= ~FE_UNDERFLOW;
            } else
#endif
#ifdef FE_INEXACT
            if (e & FE_INEXACT) {
		v = "FE_INEXACT";
                e &= ~FE_INEXACT;
            } else
#endif
#ifdef FE_INVALID
            if (e & FE_INVALID) {
		v = "FE_INVALID";
                e &= ~FE_INVALID;
            } else
#endif
            {
                snprintf(tmp, sizeof(tmp), "?? 0x%x", e);
                v = tmp;
                e = 0;
            }
#define check_add(s) do {                                               \
                if (strlen(buf[i]) + strlen(s) + 1 > sizeof(buf[i]))    \
                    printf("exception buf overflow %s + %s\n", buf[i], s); \
                else                                                    \
                    strcat(buf[i], s);                                  \
            } while(0)
            if (buf[i][0])
                check_add(" | ");
            check_add(v);
        }
        char *ret = buf[i];
        i = (i + 1) % 3;
        return ret;
}

#define scat(a,b) a ## b

/* Tests with doubles */
#ifdef PICOLIBC_DOUBLE_NOEXCEPT
#define EXCEPTION_TEST 0
#else
#define EXCEPTION_TEST	MATH_ERREXCEPT
#endif
#define DOUBLE_EXCEPTION_TEST EXCEPTION_TEST

#define BIG 1.7e308
#define SMALL 5e-324
#define FLOAT_T double
#define MIN_VAL 0x4p-1024;
#define MAX_VAL 0xf.ffffffffffff8p+1020;
#define sNAN __builtin_nans("")

#define TEST_DOUBLE

#define makemathname(s) s
#define makemathname_r(s) scat(s,_r)

#include "math_errhandling_tests.c"

#undef BIG
#undef SMALL
#undef MIN_VAL
#undef MAX_VAL
#undef sNAN
#undef makemathname
#undef makemathname_r
#undef FLOAT_T
#undef EXCEPTION_TEST
#undef TEST_DOUBLE

/* Tests with floats */
#define EXCEPTION_TEST	MATH_ERREXCEPT
#define BIG 3e38
#define SMALL 1e-45
#define MIN_VAL 0x8p-152;
#define MAX_VAL 0xf.fffffp+124;
#define sNAN __builtin_nansf("")
#define FLOAT_T float
#define TEST_FLOAT
#define makemathname(s) scat(s,f)
#define makemathname_r(s) scat(s,f_r)

#include "math_errhandling_tests.c"

int main(void)
{
	int result = 0;

#if DOUBLE_EXCEPTION_TEST
	printf("Double tests:\n");
	result += run_tests();
#endif
	printf("Float tests:\n");
	result += run_testsf();
	return result;
}
