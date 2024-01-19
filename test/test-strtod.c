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

#if defined (TINY_STDIO) || !defined(__PICOLIBC__)
#define FULL_TESTS
#endif

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wliteral-range"

static const struct {
    char        *string;
    double      dvalue;
    float       fvalue;
    long double ldvalue;
} tests[] = {
    { "1e2@", 100.0, 100.0f, 100.0l },
#ifdef FULL_TESTS
    { "0x1p-1@", 0.5, 0.5f, 0.5l },
    { "0x1p-10000000000000000000000000@", 0.0, 0.0f, 0.0l },
    { "0x1p0@", 1.0, 1.0f, 1.0l },
    { "0x10p0@", 16.0, 16.0f, 16.0l },
    { "0x1p-1023@", 0x1p-1023, 0.0f, 0x1p-1023l },
    /* Check round-to-even for floats */
    { "0x1.000002p0@", 0x1.000002p0, 0x1.000002p0f, 0x1.000002p0l },
    { "0x1.000003p0@", 0x1.000003p0, 0x1.000004p0f, 0x1.000003p0l },
    { "0x1.000001p0@", 0x1.000001p0, 0x1.000000p0f, 0x1.000001p0l },
    { "0x1.000001000000001p0@", 0x1.000001000000001p0, 0x1.000002p0f, 0x1.000001000000001p0l },
#if __SIZEOF_DOUBLE__ == 8
    /* Check round-to-even for doubles */
    { "0x100000000000008p0@", 0x1p56, 0x1p56f, 0x1.00000000000008p56l },
    { "0x100000000000008.p0@", 0x1p56, 0x1p56f, 0x1.00000000000008p56l },
    { "0x100000000000008.00p0@", 0x1p56,0x1p56f, 0x1.00000000000008p56l },
    { "0x10000000000000800p0@", 0x1p64, 0x1p64f, 0x1.00000000000008p64l },
    { "0x10000000000000801p0@", 0x1.0000000000001p64, 0x1p64f, 0x1.0000000000000801p64l },
    { "0x10000000000000800.0000000000001p0@", 0x1.0000000000001p64, 0x1p64f, 0x1.00000000000008p64l },
    { "0x10000000000001800p0@", 0x1.0000000000002p64, 0x1p64f, 0x1.00000000000018p64l },
#endif
    /* Check max values for floats */
    { "0x1.fffffep126@", 0x1.fffffep126,  0x1.fffffep126f, 0x1.fffffep126l },
    { "0x1.ffffffp126@", 0x1.ffffffp126,  0x1.000000p127f, 0x1.ffffffp126l },
    { "0x1.fffffep127@", 0x1.fffffep127,  0x1.fffffep127f, 0x1.fffffep127l },
    { "0x1.ffffffp127@", 0x1.ffffffp127, (float) INFINITY, 0x1.ffffffp127l },   /* rounds up to INFINITY for float */
#if __SIZEOF_DOUBLE__ == 8
    /* Check max values for doubles */
    { "0x1.fffffffffffffp1022@",  0x1.fffffffffffffp1022, (float) INFINITY, 0x1.fffffffffffffp1022l },
    { "0x1.fffffffffffff8p1022@", 0x1.0000000000000p1023, (float) INFINITY, 0x1.fffffffffffff8p1022l }, /* rounds up for double */
    { "0x1.fffffffffffffp1023@",  0x1.fffffffffffffp1023, (float) INFINITY, 0x1.fffffffffffffp1023l },
#endif
#if defined(_TEST_LONG_DOUBLE) && __SIZEOF_LONG_DOUBLE__ > 8
#if __LDBL_MANT_DIG__ > __DBL_MANT_DIG__
    { "0x1.fffffffffffff8p1023@",      (double) INFINITY, (float) INFINITY, 0x1.fffffffffffff8p1023l }, /* rounds up to INFINITY for double */
#else
    { "0x1.fffffffffffff8p1023@",      (double) INFINITY, (float) INFINITY, (long double) INFINITY }, /* rounds up to INFINITY for double */
#endif
    /* Check max values for long doubles */
#if __LDBL_MANT_DIG__ == 113
    { "0x1.ffffffffffffffffffffffffffffp16382@",  (double) INFINITY, (float) INFINITY, 0x1.ffffffffffffffffffffffffffffp16382l },
    { "0x1.ffffffffffffffffffffffffffff8p16382@", (double) INFINITY, (float) INFINITY, 0x1.0000000000000000000000000000p16383l }, /* rounds up for long double */
    { "0x1.ffffffffffffffffffffffffffffp16383@",  (double) INFINITY, (float) INFINITY, 0x1.ffffffffffffffffffffffffffffp16383l },
    { "0x1.ffffffffffffffffffffffffffff8p16383@", (double) INFINITY, (float) INFINITY, (long double) INFINITY },                  /* rounds up to INFINITY for long oduble */
#elif __LDBL_MANT_DIG__ == 64
    { "0x1.fffffffffffffffep16382@", (double) INFINITY, (float) INFINITY, 0x1.fffffffffffffffep16382l },
    { "0x1.ffffffffffffffffp16382@", (double) INFINITY, (float) INFINITY, 0x1.0000000000000000p16383l },        /* rounds up for long double */
    { "0x1.fffffffffffffffep16383@", (double) INFINITY, (float) INFINITY, 0x1.fffffffffffffffep16383l },
    { "0x1.ffffffffffffffffp16383@", (double) INFINITY, (float) INFINITY, (long double) INFINITY },             /* rounds up to INFINITY for long double */
#endif
#endif
#endif
};

#define NTESTS (sizeof(tests)/sizeof(tests[0]))

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
    int ret = 0;

    for (i = 0; i < (int) NTESTS; i++) {
        d = strtod(tests[i].string, &end);
        if (d != tests[i].dvalue) {
            printf("strtod(\"%s\"): got %.17e %a want %.17e %a\n", tests[i].string,
                   d, d, tests[i].dvalue, tests[i].dvalue);
            ret = 1;
        }
        if (*end != '@') {
            printf("strtod(\"%s\") end is \"%s\"\n",
                   tests[i].string, end);
            ret = 1;
        }
        f = strtof(tests[i].string, &end);
        if (f != tests[i].fvalue) {
            printf("strtof(\"%s\"): got %.17e %a want %.17e %a\n", tests[i].string,
                   (double) f, (double) f, (double) tests[i].fvalue, (double) tests[i].fvalue);
            ret = 1;
        }
        if (*end != '@') {
            printf("strtof(\"%s\") end is \"%s\"\n", tests[i].string, end);
            ret = 1;
        }
#ifdef _TEST_LONG_DOUBLE
#ifdef FULL_TESTS
        if (sizeof(long double) > sizeof(double)) {
            ld = strtold(tests[i].string, &end);
            if (ld != tests[i].ldvalue) {
                printf("strtold(\"%s\"): got %.17Le %La want %.17Le %La\n", tests[i].string,
                       ld, ld, tests[i].ldvalue, tests[i].ldvalue);
                ret = 1;
            }
            if (*end != '@') {
                printf("strtold(\"%s\") end is \"%s\"\n", tests[i].string, end);
                ret = 1;
            }
        }
#endif
#endif
    }
    return ret;
}
