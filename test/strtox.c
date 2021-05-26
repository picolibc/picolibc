/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Sebastian Meyer
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <errno.h>


#ifndef TINY_STDIO
#define printf_float(x) ((double) (x))

#ifdef _NANO_FORMATTED_IO
#ifndef NO_FLOATING_POINT
extern int _printf_float();
extern int _scanf_float();

int (*_reference_printf_float)() = _printf_float;
int (*_reference_scanf_float)() = _scanf_float;
#endif
#endif
#endif

#define inf_f  __builtin_inff()
#define inf_d  __builtin_inf()

#ifdef _LDBL_EQ_DBL
#define inf_ld __builtin_inf()
#else
#define inf_ld __builtin_infl()
#endif

int ecnt = 0;

void test_ld (int exp_errno, long double exp_result, const char* str) {
    errno = 0;
    long double result = strtold(str, (char**)0);
    if (errno == exp_errno && result == exp_result)
        return;

    ecnt++;
    printf("strtold(\"%s\"):\n  result=%Lf (expected=%Lf)\n  errno=%i (expected=%i)\n\n",
            str,
            result, exp_result,
            errno, exp_errno);
}

void test_d (int exp_errno, double exp_result, const char* str) {
    errno = 0;
    double result = strtod(str, (char**)0);
    if (errno == exp_errno && result == exp_result)
        return;

    ecnt++;
    printf("strtod(\"%s\"):\n  result=%f (expected=%f)\n  errno=%i (expected=%i)\n\n",
            str,
            result, exp_result,
            errno, exp_errno);
}

void test_f (int exp_errno, float exp_result, const char* str) {
    errno = 0;
    float result = strtof(str, (char**)0);
    if (errno == exp_errno && result == exp_result)
        return;

    ecnt++;
    printf("strtof(\"%s\"):\n  result=%f (expected=%f)\n  errno=%i (expected=%i)\n\n",
            str,
            printf_float(result), printf_float(exp_result),
            errno, exp_errno);
}

int main (void) {
    /**
     * Careful when using another libc as a reference here!
     * For example GLIBC cheats a little bit:
     *  As long as the string is prefixed with "inf" (case insensitive),
     *  the result is +/-inf. So "information" would also parse as "inf".
     */

    test_f(0,  inf_f, "inf");
    test_f(0,  inf_f, "INF");
    test_f(0,  inf_f, "infinity");
    test_f(0,  inf_f, "INFINITY");
    test_f(0, -inf_f, "-InfinitY");

    test_d(0,  inf_d, "inf");
    test_d(0,  inf_d, "INF");
    test_d(0,  inf_d, "infinity");
    test_d(0,  inf_d, "INFINITY");
    test_d(0, -inf_d, "-InfinitY");

    test_ld(0,  inf_ld, "inf");
    test_ld(0,  inf_ld, "INF");
    test_ld(0,  inf_ld, "infinity");
    test_ld(0,  inf_ld, "INFINITY");
    test_ld(0, -inf_ld, "-InfinitY");

    if (ecnt > 0) {
        printf("Errors: %i\n", ecnt);
        exit(1);
    }
    exit(0);
}
