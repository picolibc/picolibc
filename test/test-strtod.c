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

#if defined (TINY_STDIO) || !defined(__PICOLIBC__)
#define FULL_TESTS
#endif

struct {
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
    { "0x1.000001000000001p0@", 0x1.000001p0, 0x1.000002p0f, 0x1.000001000000001p0l },
    /* Check round-to-even for doubles */
    { "0x100000000000008p0@", 0x1p56, 0x1p56f, 0x1.00000000000008p56l },
    { "0x100000000000008.p0@", 0x1p56, 0x1p56f, 0x1.00000000000008p56l },
    { "0x100000000000008.00p0@", 0x1p56,0x1p56f, 0x1.00000000000008p56l },
    { "0x10000000000000800p0@", 0x1p64, 0x1p64f, 0x1.00000000000008p64l },
    { "0x10000000000000801p0@", 0x1.0000000000001p64, 0x1p64f, 0x1.0000000000000801p64l },
    { "0x10000000000000800.0000000000001p0@", 0x1.0000000000001p64, 0x1p64f, 0x1.00000000000008p64l },
    { "0x10000000000001800p0@", 0x1.0000000000002p64, 0x1p64f, 0x1.00000000000018p64l },
#endif
};

#define NTESTS (sizeof(tests)/sizeof(tests[0]))

int main(void)
{
    int i;
    double d;
    float f;
#ifdef FULL_TESTS
    long double ld;
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
#ifdef FULL_TESTS
        if (sizeof(long double) > sizeof(double)) {
            ld = strtold(tests[i].string, &end);
            if (ld != tests[i].ldvalue) {
#ifdef __PICOLIBC__
                printf("strtold(\"%s\"): got %.17e %a want %.17e %a\n", tests[i].string,
                       (double) ld, (double) ld, (double) tests[i].ldvalue, (double) tests[i].ldvalue);
#else
                printf("strtold(\"%s\"): got %.17Le %La want %.17Le %La\n", tests[i].string,
                       ld, ld, tests[i].ldvalue, tests[i].ldvalue);
#endif
                ret = 1;
            }
            if (*end != '@') {
                printf("strtold(\"%s\") end is \"%s\"\n", tests[i].string, end);
                ret = 1;
            }
        }
#endif
    }
    return ret;
}
