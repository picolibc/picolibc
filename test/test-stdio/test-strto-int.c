/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Artem Kulyk
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

/*
 * Integer strto* edge tests: signed/unsigned extremes, overflow
 * saturation with ERANGE, malformed inputs, all bases, invalid base
 * handling and end-pointer semantics.
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int errors;

static void
fail(const char *what, const char *s)
{
    printf("%s failed for \"%s\"\n", what, s);
    errors++;
}

static void
check_ul(const char *s, int base, unsigned long want, size_t consumed)
{
    char         *end;
    unsigned long got;

    errno = 0;
    end = (char *)1;
    got = strtoul(s, &end, base);
    if (got != want) {
        printf("strtoul(\"%s\", %d) got %lu want %lu\n", s, base, got, want);
        errors++;
    }
    if (end != s + consumed)
        fail("strtoul endptr", s);
    if (errno != 0)
        fail("strtoul errno", s);
}

static void
check_l(const char *s, int base, long want, size_t consumed)
{
    char *end;
    long  got;

    errno = 0;
    end = (char *)1;
    got = strtol(s, &end, base);
    if (got != want) {
        printf("strtol(\"%s\", %d) got %ld want %ld\n", s, base, got, want);
        errors++;
    }
    if (end != s + consumed)
        fail("strtol endptr", s);
    if (errno != 0)
        fail("strtol errno", s);
}

static void
check_ll(const char *s, int base, long long want, int want_errno)
{
    char     *end;
    long long got;

    errno = 0;
    got = strtoll(s, &end, base);
    if (got != want) {
        printf("strtoll(\"%s\", %d) got %lld want %lld\n", s, base, got, want);
        errors++;
    }
    if (errno != want_errno) {
        printf("strtoll(\"%s\") errno %d want %d\n", s, errno, want_errno);
        errors++;
    }
}

static void
check_ull(const char *s, int base, unsigned long long want, int want_errno)
{
    char              *end;
    unsigned long long got;

    errno = 0;
    got = strtoull(s, &end, base);
    if (got != want) {
        printf("strtoull(\"%s\", %d) got %llx want %llx\n", s, base, got, want);
        errors++;
    }
    if (errno != want_errno) {
        printf("strtoull(\"%s\") errno %d want %d\n", s, errno, want_errno);
        errors++;
    }
}

static void
test_extremes(void)
{
    char buf[32];

    check_l("0", 10, 0, 1);
    check_l("1", 10, 1, 1);
    check_l("-1", 10, -1, 2);
    check_l("+1", 10, 1, 2);

    snprintf(buf, sizeof(buf), "%ld", LONG_MAX);
    check_l(buf, 10, LONG_MAX, strlen(buf));
    snprintf(buf, sizeof(buf), "%ld", LONG_MIN);
    check_l(buf, 10, LONG_MIN, strlen(buf));

    snprintf(buf, sizeof(buf), "%lu", ULONG_MAX);
    check_ul(buf, 10, ULONG_MAX, strlen(buf));
    check_ul("0", 10, 0, 1);

    check_ll("-9223372036854775808", 10, LLONG_MIN, 0);
    check_ll("9223372036854775807", 10, LLONG_MAX, 0);
    check_ull("18446744073709551615", 10, ULLONG_MAX, 0);
    check_ull("0xffffffffffffffff", 16, ULLONG_MAX, 0);
    check_ull("0", 10, 0, 0);
}

static void
test_overflow(void)
{
    check_ll("9223372036854775808", 10, LLONG_MAX, ERANGE);
    check_ll("-9223372036854775809", 10, LLONG_MIN, ERANGE);
    check_ll("99999999999999999999999", 10, LLONG_MAX, ERANGE);
    check_ull("18446744073709551616", 10, ULLONG_MAX, ERANGE);
    check_ull("-1", 10, ULLONG_MAX, 0);
    check_ull("-18446744073709551615", 10, 1, 0);
    check_ull("-18446744073709551616", 10, ULLONG_MAX, ERANGE);
}

static void
test_malformed(void)
{
    const char   *s;
    char         *end;
    unsigned long v;

    errno = 0;
    s = "";
    end = (char *)1;
    v = strtoul(s, &end, 10);
    if (v != 0 || errno != 0)
        fail("strtoul empty", s);
    if (end != s)
        fail("strtoul empty endptr", s);

    s = "   ";
    v = strtoul(s, &end, 10);
    if (v != 0 || end != s)
        fail("strtoul spaces", s);

    s = "+";
    v = strtoul(s, &end, 10);
    if (v != 0 || end != s)
        fail("strtoul bare plus", s);

    s = "-";
    v = strtoul(s, &end, 10);
    if (v != 0 || end != s)
        fail("strtoul bare minus", s);

    s = "xyz";
    v = strtoul(s, &end, 10);
    if (v != 0 || end != s)
        fail("strtoul junk", s);

    s = "0x";
    v = strtoul(s, &end, 0);
    if (v != 0 || end != s + 1)
        fail("strtoul dangling 0x", s);

    s = "123abc";
    v = strtoul(s, &end, 10);
    if (v != 123 || end != s + 3)
        fail("strtoul trailing junk", s);

    s = "  \t-0042xyz";
    v = strtoul(s, &end, 10);
    if (v != (unsigned long)-42 || end != s + 8)
        fail("strtoul leading junk", s);

    /* NULL end pointer must be accepted */
    if (strtoul("77", NULL, 10) != 77)
        fail("strtoul NULL endptr", "77");
}

static void
test_bases(void)
{
    char *end;
    long  v;

    check_l("101", 2, 5, 3);
    check_l("777", 8, 511, 3);
    check_l("ff", 16, 255, 2);
    check_l("zz", 36, 1295, 2);
    check_l("0xff", 16, 255, 4);
    check_l("0777", 8, 511, 4);

    v = strtol("12", &end, 1);
    if (v != 0 || errno != EINVAL || end != (char *)"12")
        fail("strtol base 1", "12");

    errno = 0;
    v = strtol("12", &end, 37);
    if (v != 0 || errno != EINVAL || end != (char *)"12")
        fail("strtol base 37", "12");

    errno = 0;
    v = strtol("12", &end, -5);
    if (v != 0 || errno != EINVAL || end != (char *)"12")
        fail("strtol base -5", "12");

    /* base 0 auto-detection */
    check_l("0x10", 0, 16, 4);
    check_l("010", 0, 8, 3);
    check_l("10", 0, 10, 2);
}

int
main(void)
{
    errors = 0;
    test_extremes();
    test_overflow();
    test_malformed();
    test_bases();
    if (errors)
        printf("%d errors\n", errors);
    else
        printf("ok\n");
    return errors != 0;
}
