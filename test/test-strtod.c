/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
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
#include <math.h>
#include <string.h>
#include <wchar.h>

#if defined (__TINY_STDIO) || !defined(__PICOLIBC__)
#define FULL_TESTS
#endif

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wliteral-range"
#endif

static const struct {
    char        *string;
    double      dvalue;
    float       fvalue;
    long double ldvalue;
    char        *end_test;
} tests[] = {
    { "1e2@", 100.0, 100.0f, 100.0l, "@" },
#ifdef FULL_TESTS
#if !defined(__TINY_STDIO) || defined(__IO_FLOAT_EXACT)
    { "3752432815e-39%", 0x1.306efbp-98, 0x1.306efcp-98f, 3752432815e-39l, "%" },
    { "3.752432815e-30%", 0x1.306efbp-98, 0x1.306efcp-98f, 3752432815e-39l, "%" },
    { "3752432814e-39^", 3752432814e-39, 0x1.306efap-98f, 3752432814e-39l, "^" },
    { "3.752432814e-30^", 3752432814e-39, 0x1.306efap-98f, 3752432814e-39l, "^" },
#endif
    { "0x10.000@", 16.0, 16.0f, 16.0l, "@" },
    { "0x10.000p@", 16.0, 16.0f, 16.0l, "p@" },
    { "0x10.000p+@", 16.0, 16.0f, 16.0l, "p+@" },
    { "0x10.000p+1@", 32.0, 32.0f, 32.0l, "@" },
    { "0x10.000p-@", 16.0, 16.0f, 16.0l, "p-@" },
    { "0x10.000p-1@", 8.0, 8.0f, 8.0l, "@" },
    { "0x11.000@", 17.0, 17.0f, 17.0l, "@" },
    { "0x0.01@", 1/256.0, 1/256.0f, 1/256.0l, "@" },
    { "0x0.01p", 1/256.0, 1/256.0f, 1/256.0l, "p" },
    { "0x0.01p+", 1/256.0, 1/256.0f, 1/256.0l, "p+" },
    { "0x1p-1@", 0.5, 0.5f, 0.5l, "@" },
    { "0x1p-10000000000000000000000000@", 0.0, 0.0f, 0.0l, "@" },
    { "0x1p0@", 1.0, 1.0f, 1.0l, "@" },
    { "0x10p0@", 16.0, 16.0f, 16.0l, "@" },
    { "0x1p-1023@", 0x1p-1023, 0.0f, 0x1p-1023l, "@" },
    /* Check round-to-even for floats */
    { "0x1.000002p0@", 0x1.000002p0, 0x1.000002p0f, 0x1.000002p0l, "@" },
    { "0x1.000003p0@", 0x1.000003p0, 0x1.000004p0f, 0x1.000003p0l, "@" },
    { "0x1.000001p0@", 0x1.000001p0, 0x1.000000p0f, 0x1.000001p0l, "@" },
    { "0x1.000001000000001p0@", 0x1.000001000000001p0, 0x1.000002p0f, 0x1.000001000000001p0l, "@" },
#if __SIZEOF_DOUBLE__ == 8
    /* Check round-to-even for doubles */
    { "0x100000000000008p0@", 0x1p56, 0x1p56f, 0x1.00000000000008p56l, "@" },
    { "0x100000000000008.p0@", 0x1p56, 0x1p56f, 0x1.00000000000008p56l, "@" },
    { "0x100000000000008.00p0@", 0x1p56,0x1p56f, 0x1.00000000000008p56l, "@" },
    { "0x10000000000000800p0@", 0x1p64, 0x1p64f, 0x1.00000000000008p64l, "@" },
    { "0x10000000000000801p0@", 0x1.0000000000001p64, 0x1p64f, 0x1.0000000000000801p64l, "@" },
    { "0x10000000000000800.0000000000001p0@", 0x1.0000000000001p64, 0x1p64f, 0x1.00000000000008p64l, "@" },
    { "0x10000000000001800p0@", 0x1.0000000000002p64, 0x1p64f, 0x1.00000000000018p64l, "@" },
#endif
    /* Check max values for floats */
    { "0x1.fffffep126@", 0x1.fffffep126,  0x1.fffffep126f, 0x1.fffffep126l, "@" },
    { "0x1.ffffffp126@", 0x1.ffffffp126,  0x1.000000p127f, 0x1.ffffffp126l, "@" },
    { "0x1.fffffep127@", 0x1.fffffep127,  0x1.fffffep127f, 0x1.fffffep127l, "@" },
    { "0x1.ffffffp127@", 0x1.ffffffp127, (float) INFINITY, 0x1.ffffffp127l, "@" },   /* rounds up to INFINITY for float */
#if __SIZEOF_DOUBLE__ == 8
    /* Check max values for doubles */
    { "0x1.fffffffffffffp1022@",  0x1.fffffffffffffp1022, (float) INFINITY, 0x1.fffffffffffffp1022l, "@" },
    { "0x1.fffffffffffff8p1022@", 0x1.0000000000000p1023, (float) INFINITY, 0x1.fffffffffffff8p1022l, "@" }, /* rounds up for double */
    { "0x1.fffffffffffffp1023@",  0x1.fffffffffffffp1023, (float) INFINITY, 0x1.fffffffffffffp1023l, "@" },
#endif
#if defined(_TEST_LONG_DOUBLE) && __SIZEOF_LONG_DOUBLE__ > 8
#if __LDBL_MANT_DIG__ > __DBL_MANT_DIG__
    { "0x1.fffffffffffff8p1023@",      (double) INFINITY, (float) INFINITY, 0x1.fffffffffffff8p1023l, "@" }, /* rounds up to INFINITY for double */
#else
    { "0x1.fffffffffffff8p1023@",      (double) INFINITY, (float) INFINITY, (long double) INFINITY, "@" }, /* rounds up to INFINITY for double */
#endif
    /* Check max values for long doubles */
#if __LDBL_MANT_DIG__ == 113
    { "0x1.ffffffffffffffffffffffffffffp16382@",  (double) INFINITY, (float) INFINITY, 0x1.ffffffffffffffffffffffffffffp16382l, "@" },
    { "0x1.ffffffffffffffffffffffffffff8p16382@", (double) INFINITY, (float) INFINITY, 0x1.0000000000000000000000000000p16383l, "@" }, /* rounds up for long double */
    { "0x1.ffffffffffffffffffffffffffffp16383@",  (double) INFINITY, (float) INFINITY, 0x1.ffffffffffffffffffffffffffffp16383l, "@" },
    { "0x1.ffffffffffffffffffffffffffff8p16383@", (double) INFINITY, (float) INFINITY, (long double) INFINITY, "@" },                  /* rounds up to INFINITY for long oduble */
#elif __LDBL_MANT_DIG__ == 64
    { "0x1.fffffffffffffffep16382@", (double) INFINITY, (float) INFINITY, 0x1.fffffffffffffffep16382l, "@" },
    { "0x1.ffffffffffffffffp16382@", (double) INFINITY, (float) INFINITY, 0x1.0000000000000000p16383l, "@" },        /* rounds up for long double */
    { "0x1.fffffffffffffffep16383@", (double) INFINITY, (float) INFINITY, 0x1.fffffffffffffffep16383l, "@" },
    { "0x1.ffffffffffffffffp16383@", (double) INFINITY, (float) INFINITY, (long double) INFINITY, "@" },             /* rounds up to INFINITY for long double */
#endif
#endif
#endif
};

#define NTESTS (sizeof(tests)/sizeof(tests[0]))

static wchar_t wbuf[256], wendbuf[256];

int main(void)
{
    int i;
    double d;
    float f;
#ifdef _TEST_LONG_DOUBLE
#ifdef FULL_TESTS
    long double ld;
#endif
#endif
    char *end;
    wchar_t *wend;
    int ret = 0;

    for (i = 0; i < (int) NTESTS; i++) {
        d = strtod(tests[i].string, &end);
        if (d != tests[i].dvalue) {
            printf("strtod(\"%s\"): got %.17e %a want %.17e %a\n", tests[i].string,
                   d, d, tests[i].dvalue, tests[i].dvalue);
            ret = 1;
        }
        if (strcmp(end, tests[i].end_test) != 0) {
            printf("strtod(\"%s\") end is \"%s\" expected \"%s\"\n",
                   tests[i].string, end, tests[i].end_test);
            ret = 1;
        }

        f = strtof(tests[i].string, &end);
        if (f != tests[i].fvalue) {
            printf("strtof(\"%s\"): got %.17e %a want %.17e %a\n", tests[i].string,
                   (double) f, (double) f, (double) tests[i].fvalue, (double) tests[i].fvalue);
            ret = 1;
        }
        if (strcmp(end, tests[i].end_test) != 0) {
            printf("strtof(\"%s\") end is \"%s\" expected \"%s\"\n",
                   tests[i].string, end, tests[i].end_test);
            ret = 1;
        }
        if (tests[i].end_test[0] == '%') {
            if ((float) d != f) {
                printf("strtof(\"%s\") got %a strtod got %a\n", tests[i].string, (double) f, d);
                ret = 1;
            }
        }

        mbstowcs(wbuf, tests[i].string, sizeof(wbuf)/sizeof(wchar_t));
        mbstowcs(wendbuf, tests[i].end_test, sizeof(wendbuf)/sizeof(wchar_t));

        d = wcstod(wbuf, &wend);
        if (d != tests[i].dvalue) {
            printf("wcstod(\"%ls\"): got %.17e %a want %.17e %a\n", wbuf,
                   d, d, tests[i].dvalue, tests[i].dvalue);
            ret = 1;
        }
        if (wcscmp(wend, wendbuf) != 0) {
            printf("wcstod(\"%ls\") end is \"%ls\" expected \"%ls\"\n",
                   wbuf, wend, wendbuf);
            ret = 1;
        }

        f = wcstof(wbuf, &wend);
        if (f != tests[i].fvalue) {
            printf("wcstof(\"%ls\"): got %.17e %a want %.17e %a\n", wbuf,
                   (double) f, (double) f, (double) tests[i].fvalue, (double) tests[i].fvalue);
            ret = 1;
        }
        if (wcscmp(wend, wendbuf) != 0) {
            printf("wcstof(\"%ls\") end is \"%ls\" expected \"%ls\"\n",
                   wbuf, wend, wendbuf);
            ret = 1;
        }
        if (tests[i].end_test[0] == '%') {
            if ((float) d != f) {
                printf("wcstof(\"%ls\") got %a wcstod got %a\n", wbuf, (double) f, d);
                ret = 1;
            }
        }

#ifdef _TEST_LONG_DOUBLE
#ifdef FULL_TESTS
        if (sizeof(long double) > sizeof(double)
            && tests[i].end_test[0] != '%'
            && tests[i].end_test[0] != '^')
        {
            ld = strtold(tests[i].string, &end);
            if (ld != tests[i].ldvalue) {
                printf("strtold(\"%s\"): got %.17Le %La want %.17Le %La\n", tests[i].string,
                       ld, ld, tests[i].ldvalue, tests[i].ldvalue);
                ret = 1;
            }
            if (strcmp(end, tests[i].end_test) != 0) {
                printf("strtold(\"%s\") end is \"%s\" expected \"%s\"\n",
                       tests[i].string, end, tests[i].end_test);
                ret = 1;
            }

            ld = wcstold(wbuf, &wend);
            if (ld != tests[i].ldvalue) {
                printf("wcstold(\"%ls\"): got %.17Le %La want %.17Le %La\n", wbuf,
                       ld, ld, tests[i].ldvalue, tests[i].ldvalue);
                ret = 1;
            }
            if (wcscmp(wend, wendbuf) != 0) {
                printf("wcstold(\"%ls\") end is \"%ls\" expected \"%ls\"\n",
                       wbuf, wend, wendbuf);
                ret = 1;
            }

        }
#endif
#endif
    }
    return ret;
}
