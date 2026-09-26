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
 * Integer printf edge tests: signed/unsigned extremes for every
 * integer conversion, buffer truncation limits and snprintf return
 * values.  Expected strings are produced by an independent reference
 * converter.
 */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define REF_MAX 32

static int errors;

/*
 * Format the magnitude of v (negated when neg) in the given base.
 * This is a deliberately naive reference implementation.
 */
static void
ref_ull(unsigned long long mag, int base, int neg, char *out)
{
    char tmp[REF_MAX];
    int  i = 0, j = 0;

    do {
        unsigned d = (unsigned)(mag % (unsigned)base);
        tmp[i++] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
        mag /= (unsigned)base;
    } while (mag);
    if (neg)
        tmp[i++] = '-';
    while (i)
        out[j++] = tmp[--i];
    out[j] = '\0';
}

static void
compare(const char *what, const char *got, int n, const char *want)
{
    if (n != (int)strlen(want) || strcmp(got, want) != 0) {
        printf("%s: got \"%s\" (%d) want \"%s\" (%d)\n", what, got, n, want, (int)strlen(want));
        errors++;
    }
}

static void
check_ull(const char *what, const char *fmt, unsigned long long v, int neg, int base,
          unsigned long long mag)
{
    char want[REF_MAX];
    char got[REF_MAX + 8];

    ref_ull(mag, base, neg, want);
    compare(what, got, snprintf(got, sizeof(got), fmt, v), want);
}

static void
check_ul(const char *what, const char *fmt, unsigned long v, int neg, int base, unsigned long mag)
{
    char want[REF_MAX];
    char got[REF_MAX + 8];

    ref_ull(mag, base, neg, want);
    compare(what, got, snprintf(got, sizeof(got), fmt, v), want);
}

static void
check_ll(const char *what, const char *fmt, long long v, int neg, int base, unsigned long long mag)
{
    char want[REF_MAX];
    char got[REF_MAX + 8];

    ref_ull(mag, base, neg, want);
    compare(what, got, snprintf(got, sizeof(got), fmt, v), want);
}

static void
check_l(const char *what, const char *fmt, long v, int neg, int base, unsigned long mag)
{
    char want[REF_MAX];
    char got[REF_MAX + 8];

    ref_ull(mag, base, neg, want);
    compare(what, got, snprintf(got, sizeof(got), fmt, v), want);
}

static void
test_extremes(void)
{
#ifdef _HAS_IO_LONG_LONG
    /* unsigned long long extremes */
    check_ull("ull max", "%llu", ULLONG_MAX, 0, 10, ULLONG_MAX);
    check_ull("ull zero", "%llu", 0, 0, 10, 0);
    check_ull("ull one", "%llu", 1, 0, 10, 1);
    check_ull("ullx max", "%llx", ULLONG_MAX, 0, 16, ULLONG_MAX);
    check_ull("ullx 2^63", "%llx", 0x8000000000000000ULL, 0, 16, 0x8000000000000000ULL);

    /* values around powers of ten exercise the div-by-10 estimate */
    check_ull("ull 10", "%llu", 10, 0, 10, 10);
    check_ull("ull 99", "%llu", 99, 0, 10, 99);
    check_ull("ull 100", "%llu", 100, 0, 10, 100);
    check_ull("ull 1e19-1", "%llu", 9999999999999999999ULL, 0, 10, 9999999999999999999ULL);
    check_ull("ull 1e19", "%llu", 10000000000000000000ULL, 0, 10, 10000000000000000000ULL);
    check_ull("ull 123..", "%llu", 1234567890123456789ULL, 0, 10, 1234567890123456789ULL);

    /* signed long long extremes */
    check_ll("ll min", "%lld", LLONG_MIN, 1, 10,
             (unsigned long long)0 - (unsigned long long)LLONG_MIN);
    check_ll("ll max", "%lld", LLONG_MAX, 0, 10, (unsigned long long)LLONG_MAX);
    check_ll("ll -1", "%lld", -1, 1, 10, 1);
#endif

    /* 32/64-bit long extremes (width follows the target) */
    check_l("l max", "%ld", LONG_MAX, 0, 10, (unsigned long)LONG_MAX);
    check_l("l min", "%ld", LONG_MIN, 1, 10, (unsigned long)0 - (unsigned long)LONG_MIN);
    check_ul("ul max", "%lu", ULONG_MAX, 0, 10, ULONG_MAX);
    check_ul("ul zero", "%lu", 0, 0, 10, 0);
    check_ul("ul octal", "%lo", 01234567UL, 0, 8, 01234567UL);
}

static void
test_truncation(void)
{
    char buf[24];
    char want[REF_MAX];
    int  n;
    int  len;

    /* return value is the length which would have been written */
    n = snprintf(buf, 4, "%d", 123456789);
    if (n != 9 || strcmp(buf, "123") != 0) {
        printf("truncate 4: got \"%s\" (%d)\n", buf, n);
        errors++;
    }

    n = snprintf(buf, 1, "%d", 42);
    if (n != 2 || buf[0] != '\0') {
        printf("truncate 1: got \"%s\" (%d)\n", buf, n);
        errors++;
    }

    n = snprintf(buf, 0, "%d", 42);
    if (n != 2) {
        printf("truncate 0: got (%d)\n", n);
        errors++;
    }

    ref_ull(ULONG_MAX, 10, 0, want);
    len = (int)strlen(want);

    n = snprintf(buf, sizeof(buf), "%lu", (unsigned long)ULONG_MAX);
    if (n != len || strcmp(buf, want) != 0) {
        printf("no truncate: got \"%s\" (%d)\n", buf, n);
        errors++;
    }

    /* exact fit: length plus terminator fills the buffer */
    n = snprintf(buf, (size_t)len + 1, "%lu", (unsigned long)ULONG_MAX);
    if (n != len || strcmp(buf, want) != 0) {
        printf("exact fit: got \"%s\" (%d)\n", buf, n);
        errors++;
    }

    /* one byte short of the exact fit */
    n = snprintf(buf, (size_t)len, "%lu", (unsigned long)ULONG_MAX);
    if (n != len || (int)strlen(buf) != len - 1 || strncmp(buf, want, (size_t)len - 1) != 0) {
        printf("short fit: got \"%s\" (%d)\n", buf, n);
        errors++;
    }
}

int
main(void)
{
    errors = 0;
    test_extremes();
    test_truncation();
    if (errors)
        printf("%d errors\n", errors);
    else
        printf("ok\n");
    return errors != 0;
}
