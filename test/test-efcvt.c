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

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct test {
    double      value;
    int         ndigit;
    int         decpt;
    int         sign;
    char        *expect;
};

const struct test ecvt_tests[] = {
    { 1.0e0,    4,   1, 0, "1000", },
    { -1.0e0,   4,   1, 1, "1000", },
    { 1.0e7,    7,   8, 0, "1000000" },
    { 1.0e-12,  4, -11, 0, "1000" },
};

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wliteral-range"

#if !defined(TINY_STDIO) && !defined(NO_NEWLIB)
#define ecvt_r(n, dig, dec, sign, buf, len) (ecvtbuf(n, dig, dec, sign, buf) ? 0 : -1)
#define fcvt_r(n, dig, dec, sign, buf, len) (fcvtbuf(n, dig, dec, sign, buf) ? 0 : -1)
#define ecvtf_r(n, dig, dec, sign, buf, len) (ecvtbuf(n, dig, dec, sign, buf) ? 0 : -1)
#define fcvtf_r(n, dig, dec, sign, buf, len) (fcvtbuf(n, dig, dec, sign, buf) ? 0 : -1)
#define SKIP_FCVT_NEG
#endif

#define N_ECVT_TESTS    (sizeof(ecvt_tests) / sizeof(ecvt_tests[0]))

const struct test fcvt_tests[] = {
    { 0.0058882729652625027, 7, -2, 0, "58883", },
    { 1.0e0,    4,   1, 0, "10000", },
    { -1.0e0,   4,   1, 1, "10000", },
    { 1.0e7,    7,   8, 0, "100000000000000" },
    { 1.0e-12,  4,  -4, 0, "" },
#ifndef SKIP_FCVT_NEG
    /* legacy newlib doesn't handle negative ndecimal correctly */
    { 1.23456e0,-4,  1, 0, "1" },
    { 1.23456e1,-4,  2, 0, "10" },
    { 1.23456e2,-4,  3, 0, "100" },
    { 1.23456e3,-4,  4, 0, "1000" },
    { 1.23456e4,-4,  5, 0, "10000" },
    { 1.23456e5,-4,  6, 0, "120000" },
    { 1.23456e6,-4,  7, 0, "1230000" },
#endif
    { 1.23456e-10,4,-4, 0, "" },
    { 1.23456e-9,4, -4, 0, "" },
    { 1.23456e-8,4, -4, 0, "" },
    { 1.23456e-7,4, -4, 0, "" },
    { 1.23456e-6,4, -4, 0, "" },
    { 1.23456e-5,4, -4, 0, "" },
    { 1.23456e-4,4, -3, 0, "1" },
    { 1.23456e-3,4, -2, 0, "12" },
    { 1.23456e-2,4, -1, 0, "123" },
    { 1.23456e-1,4,  0, 0, "1235" },
    { 1.23456e0,4,   1, 0, "12346" },
    { 1.23456e1,4,   2, 0, "123456" },
    { 1.23456e2,4,   3, 0, "1234560" },
    { 1.23456e3,4,   4, 0, "12345600" },
    { 1.23456e4,4,   5, 0, "123456000" },
    { 1.23456e5,4,   6, 0, "1234560000" },
    { 1.23456e6,4,   7, 0, "12345600000" },
    { 1.23456e7,4,   8, 0, "123456000000" },
    { 1.23456e8,4,   9, 0, "1234560000000" },
    { 1.23456e9,4,  10, 0, "12345600000000" },
};

#define N_FCVT_TESTS    (sizeof(fcvt_tests) / sizeof(fcvt_tests[0]))

const struct test fcvt_extra_tests[] = {
#ifdef TINY_STDIO
#if __SIZEOF_DOUBLE__ == 4
    { 0x1.fffffep127, 9, 39, 0, "340282350000000000000000000000000000000000000000" },
#else
    { 0x1.fffffffffffffp1023, 17, 309, 0, "17976931348623157"
      "0000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000"
      "000000000000"
      "00000000000000000"},
    { 0x1.fffffep127, 9, 39, 0, "340282346638528860000000000000000000000000000000" },
#endif
#else
    { 0x1.fffffffffffffp1023, 17, 309, 0, "17976931348623157"
      "0814527423731704356798070567525844996598"
      "9174768031572607800285387605895586327668"
      "7817154045895351438246423432132688946418"
      "2768467546703537516986049910576551282076"
      "2454900903893289440758685084551339423045"
      "8323690322294816580855933212334827479782"
      "6204144723168738177180919299881250404026"
      "184124858368"
      "00000000000000000"},
    { 0x1.fffffep127, 9, 39, 0, "340282346638528859811704183484516925440000000000" },
#endif
};

#define N_FCVT_EXTRA_TESTS      (sizeof(fcvt_extra_tests) / sizeof(fcvt_extra_tests[0]))

const struct test fcvtf_tests[] = {
#ifdef TINY_STDIO
    { 0x1.fffffep127, 9, 39, 0, "340282350000000000000000000000000000000000000000" },
#else
    /* legacy newlib uses the double path for this operation */
    { 0x1.fffffep127, 9, 39, 0, "340282346638528859811704183484516925440000000000" },
#endif
};

#define N_FCVTF_TESTS    (sizeof(fcvtf_tests) / sizeof(fcvtf_tests[0]))

#if defined(TINY_STDIO) && !defined(_IO_FLOAT_EXACT)
/* non-exact tinystdio conversions are not precise over about 6 digits */
#define SKIP_LONG_FLOAT 1
#else
#define SKIP_LONG_FLOAT 0
#endif

#if (!defined(TINY_STDIO) || !defined(_IO_FLOAT_EXACT)) && __SIZEOF_DOUBLE__ < 8
#undef SKIP_LONG_FLOAT
#define SKIP_LONG_FLOAT 1
#define SKIP_LONGISH_FLOAT 1
#endif

#ifndef SKIP_LONGISH_FLOAT
#define SKIP_LONGISH_FLOAT 0
#endif

#define many_tests(tests, func, n, skip_long) do {                      \
    for (i = 0; i < n; i++) {                                           \
        int decpt;                                                      \
        int sign;                                                       \
        int ret = func(tests[i].value, tests[i].ndigit, &decpt, &sign, buf, sizeof(buf)); \
                                                                        \
        if (ret < 0) {                                                  \
            printf("%d: failed\n", ret);                                \
            continue;                                                   \
        }                                                               \
        if (skip_long && strlen(buf) > 6) {                             \
            printf("skipping test as result is long (%s)\n", buf);      \
            continue;                                                   \
        }                                                               \
        if (strcmp(buf, tests[i].expect) != 0 ||                        \
            decpt != tests[i].decpt ||                                  \
            sign != tests[i].sign)                                      \
        {                                                               \
            printf(#func ":%d got '%s' dec %d sign %d expect '%s' dec %d sign %d\n", \
                   i, buf, decpt, sign,                                 \
                   tests[i].expect, tests[i].decpt, tests[i].sign);     \
            error = 1;                                                  \
        }                                                               \
    }                                                                   \
    } while(0)

int
main(void)
{
    int error = 0;
    unsigned i;
    static char buf[2048];

    many_tests(fcvt_tests, fcvt_r, N_FCVT_TESTS, SKIP_LONGISH_FLOAT);
    many_tests(fcvt_extra_tests, fcvt_r, N_FCVT_EXTRA_TESTS, SKIP_LONG_FLOAT);
    many_tests(ecvt_tests, ecvt_r, N_ECVT_TESTS, SKIP_LONGISH_FLOAT);
#ifdef __PICOLIBC__
    many_tests(fcvt_tests, fcvtf_r, N_FCVT_TESTS, SKIP_LONG_FLOAT);
    many_tests(fcvtf_tests, fcvtf_r, N_FCVTF_TESTS, SKIP_LONG_FLOAT);
    many_tests(ecvt_tests, ecvtf_r, N_ECVT_TESTS, SKIP_LONGISH_FLOAT);
#endif
    return error;
}
