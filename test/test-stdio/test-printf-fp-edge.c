/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Artem Kulyk
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
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Floating-point printf edge tests for the decimal digit loops:
 * zeroes (including signed zero), halfway rounding, subnormals,
 * extreme magnitudes, high precision and buffer truncation.
 * Expected strings are the correctly rounded decimal representations
 * of the values as converted by the active variant's converter —
 * binary64 for the double variant, binary32 for the float variant.
 * The integer-only variants skip.
 */

#include <stdio.h>
#include <string.h>

#ifndef __PICOLIBC__
#define _HAS_IO_DOUBLE
#define printf_float(x) ((double)(x))
#endif

#if defined(_HAS_IO_DOUBLE) || defined(_HAS_IO_FLOAT)

static int errors;

static void
t(int line, const char *want, const char *fmt, double v)
{
    char buf[512];

    snprintf(buf, sizeof(buf), fmt, printf_float(v));
    if (strcmp(buf, want) != 0) {
        printf("%d: %s: got \"%s\" want \"%s\"\n", line, fmt, buf, want);
        errors++;
    }
}

#if defined(_HAS_IO_DOUBLE)
/*
 * Exact decimal expansions (or correctly rounded results) for
 * binary64.  These hold only for the double variant; float32 rounds
 * differently.
 */
static void
test_values(void)
{
    /* zero and sign of zero */
    t(__LINE__, "0", "%g", 0.0);
    t(__LINE__, "-0", "%g", -0.0);
    t(__LINE__, "0.00", "%.2f", 0.0);
    t(__LINE__, "-0.00", "%.2f", -0.0);
    t(__LINE__, "0e+00", "%.0e", 0.0);
    t(__LINE__, "-0e+00", "%.0e", -0.0);

    /*
     * Halfway rounding to integer requires IEEE 754 round-half-even.
     * The approximate converter (io-float-exact=false) does not
     * implement that rule, so these cases run only with the exact
     * converter (or a reference libc).  The %g cases below are exact
     * decimal representations and need no rounding.
     */
#if !defined(__PICOLIBC__) || defined(__IO_FLOAT_EXACT)
    t(__LINE__, "0", "%.0f", 0.5);
    t(__LINE__, "2", "%.0f", 1.5);
    t(__LINE__, "2", "%.0f", 2.5);
    t(__LINE__, "1000000", "%.0f", 999999.5);
#endif
    t(__LINE__, "0.5", "%g", 0.5);
    t(__LINE__, "2.5", "%g", 2.5);

    /* plain and rounded fractions */
    t(__LINE__, "1", "%g", 1.0);
    t(__LINE__, "0.10", "%.2f", 0.1);
    t(__LINE__, "3.1415926536", "%.10f", 3.14159265358979);
    t(__LINE__, "2.71828182846", "%.12g", 2.71828182845905);

    /* extreme magnitudes */
    t(__LINE__, "1e-05", "%g", 1e-5);
    t(__LINE__, "1e-300", "%g", 1e-300);
    t(__LINE__, "1e+300", "%g", 1e300);
    t(__LINE__, "123456789", "%.0f", 123456789.0);

    /* subnormals and the smallest positive binary64 */
    t(__LINE__, "5e-324", "%.0e", 5e-324);
    t(__LINE__, "1e-323", "%.0e", 1e-323);
    t(__LINE__, "2.225e-308", "%.3e", 2.2250738585072014e-308);
    t(__LINE__, "1.798e+308", "%.3e", 1.7976931348623157e308);
    /* 1e21 is exactly representable; an exact %.0f expansion of
     * 9.99999e21 needs 22 digits, beyond the converters' 17-digit
     * limit, so it is out of scope here. */
    t(__LINE__, "1000000000000000000000", "%.0f", 1e21);
}

#elif defined(_HAS_IO_FLOAT)
/*
 * Correctly rounded decimal representations of the binary32 values.
 * Literals are written as binary64; printf_float narrows them to
 * binary32 exactly as the C compiler would.
 */
static void
test_values(void)
{
    /* zero and sign of zero */
    t(__LINE__, "0", "%g", 0.0);
    t(__LINE__, "-0", "%g", -0.0);
    t(__LINE__, "0.00", "%.2f", 0.0);
    t(__LINE__, "-0.00", "%.2f", -0.0);
    t(__LINE__, "0e+00", "%.0e", 0.0);
    t(__LINE__, "-0e+00", "%.0e", -0.0);

    /*
     * Halfway rounding to integer requires IEEE 754 round-half-even;
     * the approximate converter (io-float-exact=false) does not
     * implement that rule.  The %g cases are exact representations.
     */
    t(__LINE__, "0.5", "%g", 0.5);
    t(__LINE__, "2.5", "%g", 2.5);
#if !defined(__PICOLIBC__) || defined(__IO_FLOAT_EXACT)
    t(__LINE__, "2", "%.0f", 1.5);
    t(__LINE__, "2", "%.0f", 2.5);
    t(__LINE__, "1000000", "%.0f", 999999.5);
#endif

    /* fractions */
    t(__LINE__, "0.10", "%.2f", 0.1);
    t(__LINE__, "0.1", "%g", 0.1);
    t(__LINE__, "3.14159", "%.5f", 3.14159265);
    t(__LINE__, "1", "%g", 1.0);

    /* magnitudes */
    t(__LINE__, "1e-05", "%g", 1e-5);
    t(__LINE__, "1e-30", "%g", 1e-30);
    t(__LINE__, "1e+30", "%g", 1e30);
    t(__LINE__, "1234567", "%.0f", 1234567.0);

    /* subnormals and extremes of binary32 */
    t(__LINE__, "1e-45", "%.0e", 1.401298464324817e-45);
    t(__LINE__, "1.175e-38", "%.3e", 1.1754943508222875e-38);
    t(__LINE__, "3.403e+38", "%.3e", 3.4028234663852886e38);
}
#endif

static void
test_truncation(void)
{
    char buf[8];
    int  n;

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
    n = snprintf(buf, (size_t)3, "%g", printf_float(3.14159));
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
    if (n != 7 || strcmp(buf, "3.") != 0) {
        printf("trunc %%g: got \"%s\" (%d)\n", buf, n);
        errors++;
    }

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
    n = snprintf(buf, (size_t)1, "%.10f", printf_float(2.71828182845905));
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
    if (n != 12 || strcmp(buf, "") != 0) {
        printf("trunc %%.10f: got \"%s\" (%d)\n", buf, n);
        errors++;
    }

#if defined(_HAS_IO_FLOAT)
    /* The approximate converter exhausts its decimal places before
     * producing nine digits for the smallest subnormal. It must finish
     * padding rather than loop with a zero decimal divisor, even when
     * the output buffer is empty. Do not require exact-converter digits.
     */
    n = snprintf(NULL, 0, "%.8e", printf_float(0x1p-149f));
    if (n != 14) {
        printf("subnormal %%.8e: got length %d, want 14\n", n);
        errors++;
    }
#endif
}

int
main(void)
{
    errors = 0;
    test_values();
    test_truncation();
    if (errors)
        printf("%d errors\n", errors);
    else
        printf("ok\n");
    return errors != 0;
}

#else /* !(_HAS_IO_DOUBLE || _HAS_IO_FLOAT) */

int
main(void)
{
    printf("floating-point printf not supported, test skipped\n");
    return 77;
}

#endif
