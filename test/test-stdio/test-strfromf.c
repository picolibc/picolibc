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

#define __STDC_WANT_IEC_60559_BFP_EXT__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct test {
    const char *format;
    double      value;
    const char *value_text; /* printed without needing float printf */
    const char *expected;
};

#define TEST(format, value, expected) { format, value, #value, expected }

static const struct test tests[] = {
    TEST("%.0f", 999.9, "1000"),        TEST("%.1f", 99.96, "100.0"),
    TEST("%.3f", 9.9996, "10.000"),

    TEST("%.0f", .9999999, "1"),        TEST("%.1f", .9999999, "1.0"),
    TEST("%.2f", .9999999, "1.00"),     TEST("%.3f", .9999999, "1.000"),
    TEST("%.4f", .9999999, "1.0000"),   TEST("%.5f", .9999999, "1.00000"),
    TEST("%.6f", .9999999, "1.000000"), TEST("%.7f", .9999999, "0.9999999"),

    TEST("%.0f", 9.999999, "10"),       TEST("%.1f", 9.999999, "10.0"),
    TEST("%.2f", 9.999999, "10.00"),    TEST("%.3f", 9.999999, "10.000"),
    TEST("%.4f", 9.999999, "10.0000"),  TEST("%.5f", 9.999999, "10.00000"),
    TEST("%.6f", 9.999999, "9.999999"),

    TEST("%.0f", 99.99999, "100"),      TEST("%.1f", 99.99999, "100.0"),
    TEST("%.2f", 99.99999, "100.00"),   TEST("%.3f", 99.99999, "100.000"),
    TEST("%.4f", 99.99999, "100.0000"), TEST("%.5f", 99.99999, "99.99999"),

    TEST("%.0f", 999.9999, "1000"),     TEST("%.1f", 999.9999, "1000.0"),
    TEST("%.2f", 999.9999, "1000.00"),  TEST("%.3f", 999.9999, "1000.000"),
    TEST("%.4f", 999.9999, "999.9999"),

    TEST("%.0f", 9999.999, "10000"),    TEST("%.1f", 9999.999, "10000.0"),
    TEST("%.2f", 9999.999, "10000.00"), TEST("%.3f", 9999.999, "9999.999"),

    TEST("%.0f", 99999.99, "100000"),   TEST("%.1f", 99999.99, "100000.0"),
    TEST("%.2f", 99999.99, "99999.99"),

    TEST("%.0f", 999999.9, "1000000"),  TEST("%.1f", 999999.9, "999999.9"),

    TEST("%.0f", 9999999., "9999999"),
};

#define SENTINAL 0xa5

int
main(void)
{
    int failures = 0;

    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        const struct test *t = &tests[i];
        char               buf[32];
        int                ret;

        memset(buf, SENTINAL, sizeof(buf));

        ret = strfromf(buf, 16, t->format, (float)t->value);

        if (strcmp(buf, t->expected) == 0 && ret == (int)strlen(buf)
            && buf[ret + 1] == (char)SENTINAL) {
            ;
        } else {
            printf("FAIL \"%s\" %s: got \"%s\" (returned %d), expected \"%s\"\n", t->format,
                   t->value_text, buf, ret, t->expected);
            failures = 1;
        }

        ret = strfromd(buf, 16, t->format, t->value);

        if (strcmp(buf, t->expected) == 0 && ret == (int)strlen(buf)
            && buf[ret + 1] == (char)SENTINAL) {
            ;
        } else {
            printf("FAIL \"%s\" %s: got \"%s\" (returned %d), expected \"%s\"\n", t->format,
                   t->value_text, buf, ret, t->expected);
            failures = 1;
        }
    }
    return failures;
}
