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

// count errors
int ecnt = 0;

// see `main` for documentation on the test_XX parameters.
#define TEST_TMPL(_NAME_, _STRTOXX_, _TYPE_, _FMT_, _PRINTF_FLOAT_) \
void _NAME_ (int exp_errno, _TYPE_ exp_result, int exp_chars_consumed, const char *str) { \
    errno = 0; \
    char *endptr = NULL; \
\
    _TYPE_ result = _STRTOXX_(str, exp_chars_consumed >= 0 ? &endptr : (char**)0); \
    if (errno == exp_errno && result == exp_result) { \
        if (exp_chars_consumed < 0) \
            return; \
        if (str + exp_chars_consumed == endptr) \
            return; \
    } \
\
    ecnt++; \
    printf(# _STRTOXX_ "(\"%s\"):\n  result=" _FMT_ " (expected=" _FMT_ ")\n  errno=%i (expected=%i)\n", \
            str, \
            _PRINTF_FLOAT_(result), _PRINTF_FLOAT_(exp_result), \
            errno, exp_errno); \
    if (exp_chars_consumed >= 0 && endptr) \
        printf("  endptr='%s'\n  chars_consumed=%i (expected=%i)\n\n", \
                endptr, \
                endptr - str, \
                exp_chars_consumed \
        ); \
    else if (exp_chars_consumed >= 0) \
        printf("  endptr=(NULL)\n  chars_consumed=0 (expected=%i)\n\n", exp_chars_consumed); \
    else \
        printf("\n"); \
}

TEST_TMPL(test_f, strtof, float, "%f", printf_float)
TEST_TMPL(test_d, strtod, double, "%f", )
TEST_TMPL(test_ld, strtold, long double, "%Lf", )

int main (void) {

    /**
     * The test_XX take four parameters:
     *  `exp_errno` is the expected errno
     *  `exp_result` is the expected result
     *  `exp_chars_consumed` is the number of characters that should be read
     *  `str` is the input string to the function under test
     *
     * The third parameter `exp_chars_consumed` is a bit special:
     *  strtoXX has an optional parameter `char **endptr`. If `exp_chars_consumed` is negative,
     *  the `endptr` is set to `(char**)0`. If a non-negative number (including 0) is passed,
     *  the `endptr` is expected to point to `str + exp_chars_consumed`.
     * So for `str`="information", `endptr` should point to the postfix "ormation", hence
     * the correct value for `exp_chars_consumed` would be 3.
     */

    test_f(0,  inf_f,  3, "iNformation");
    test_f(0,  inf_f, -1, "INF");
    test_f(0,  inf_f,  8, "infinity");
    test_f(0,  inf_f, -1, "INFINITY");
    test_f(0, -inf_f, -1, "-InfinitY");
    test_f(0, -inf_f,  9, "-InfinitY and beyond");
    test_f(0,  inf_f,  9, " InfinitY and beyond");

    test_d(0,  inf_d,  3, "iNformation");
    test_d(0,  inf_d, -1, "INF");
    test_d(0,  inf_d,  8, "infinity");
    test_d(0,  inf_d, -1, "INFINITY");
    test_d(0, -inf_d, -1, "-InfinitY");
    test_d(0, -inf_d,  9, "-InfinitY and Beyond");
    test_d(0,  inf_d,  9, " InfinitY and BEyond");

    test_ld(0,  inf_ld,  3, "iNformation");
    test_ld(0,  inf_ld, -1, "INF");
    test_ld(0,  inf_ld,  8, "infinity");
    test_ld(0,  inf_ld, -1, "INFINITY");
    test_ld(0, -inf_ld, -1, "-InfinitY");
    test_ld(0, -inf_ld,  9, "-InfinitY and beyoND");
    test_ld(0,  inf_ld,  9, " InfinitY and beyonD");

    if (ecnt > 0) {
        printf("Errors: %i\n", ecnt);
        exit(1);
    }
    exit(0);
}
