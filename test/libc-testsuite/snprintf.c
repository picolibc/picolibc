/*
 * Copyright © 2005-2020 Rich Felker
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-truncation"
#pragma GCC diagnostic ignored "-Woverflow"

#define DISABLE_SLOW_TESTS

#define TEST(r, f, x, m) ( \
((r) = (f)) == (x) || \
(printf(__FILE__ ":%d: %s failed (" m ")\n", __LINE__, #f, r, x), err++, 0) )

#define TEST_S(s, x, m) ( \
!strcmp((s),(x)) || \
(printf(__FILE__ ":%d: [%s] != [%s] (%s)\n", __LINE__, s, x, m), err++, 0) )

static const struct {
	const char *fmt;
	int i;
	const char *expect;
} int_tests[] = {
	/* width, precision, alignment */
	{ "%04d", 12, "0012" },
	{ "%.3d", 12, "012" },
	{ "%3d", 12, " 12" },
	{ "%-3d", 12, "12 " },
	{ "%+3d", 12, "+12" },
	{ "%+-5d", 12, "+12  " },
	{ "%+- 5d", 12, "+12  " },
	{ "%- 5d", 12, " 12  " },
	{ "% d", 12, " 12" },
	{ "%0-5d", 12, "12   " },
	{ "%-05d", 12, "12   " },

	/* ...explicit precision of 0 shall be no characters. */
	{ "%.0d", 0, "" },
	{ "%.0o", 0, "" },
	{ "%#.0d", 0, "" },
#ifdef TINY_STDIO
	{ "%#.0o", 0, "" },
#endif
	{ "%#.0x", 0, "" },

	/* ...but it still has to honor width and flags. */
	{ "%2.0u", 0, "  " },
	{ "%02.0u", 0, "  " },
	{ "%2.0d", 0, "  " },
	{ "%02.0d", 0, "  " },
	{ "% .0d", 0, " " },
	{ "%+.0d", 0, "+" },

	/* hex: test alt form and case */
	{ "%x", 63, "3f" },
	{ "%#x", 63, "0x3f" },
	{ "%X", 63, "3F" },

	/* octal: test alt form */
	{ "%o", 15, "17" },
	{ "%#o", 15, "017" },

	{ NULL, 0.0, NULL }
};

static const struct {
	const char *fmt;
	double f;
	const char *expect;
} fp_tests[] = {
	/* basic form, handling of exponent/precision for 0 */
	{ "%e", 0.0, "0.000000e+00" },
	{ "%f", 0.0, "0.000000" },
	{ "%g", 0.0, "0" },
	{ "%#g", 0.0, "0.00000" },

	/* rounding */
	{ "%f", 1.1, "1.100000" },
	{ "%f", 1.2, "1.200000" },
	{ "%f", 1.3, "1.300000" },
	{ "%f", 1.4, "1.400000" },
	{ "%f", 1.5, "1.500000" },
	{ "%.4f", 1.06125, "1.0613" },
	{ "%.2f", 1.375, "1.38" },
	{ "%.1f", 1.375, "1.4" },
	{ "%.15f", 1.1, "1.100000000000000" },
#ifdef TINY_STDIO
	{ "%.16f", 1.1, "1.1000000000000000" },
	{ "%.17f", 1.1, "1.10000000000000000" },
#else
        /* legacy stdio adds non-zero digits beyond needed precision */
	{ "%.16f", 1.1, "1.1000000000000001" },
	{ "%.17f", 1.1, "1.10000000000000009" },
#endif
	{ "%.2e", 1500001.0, "1.50e+06" },
	{ "%.2e", 1505000.0, "1.50e+06" },
	{ "%.2e", 1505000.00000095367431640625, "1.51e+06" },
	{ "%.2e", 1505001.0, "1.51e+06" },
	{ "%.2e", 1506000.0, "1.51e+06" },

	/* correctness in DBL_DIG places */
	{ "%.15g", 1.23456789012345, "1.23456789012345" },

	/* correct choice of notation for %g */
	{ "%g", 0.0001, "0.0001" },
	{ "%g", 0.00001, "1e-05" },
	{ "%g", 123456, "123456" },
	{ "%g", 1234567, "1.23457e+06" },
	{ "%.7g", 1234567, "1234567" },
	{ "%.7g", 12345678, "1.234568e+07" },
	{ "%.8g", 0.1, "0.1" },
	{ "%.9g", 0.1, "0.1" },
	{ "%.10g", 0.1, "0.1" },
	{ "%.11g", 0.1, "0.1" },

	/* pi in double precision, printed to a few extra places */
	{ "%.15f", M_PI, "3.141592653589793" },
#ifdef TINY_STDIO
	{ "%.18f", M_PI, "3.141592653589793000" },
#else
        /* legacy stdio adds non-zero digits beyond needed precision */
	{ "%.18f", M_PI, "3.141592653589793116" },
#endif

	/* exact conversion of large integers */
#ifdef TINY_STDIO
	{ "%.0f", 340282366920938463463374607431768211456.0,
	         "340282366920938500000000000000000000000" },
#else
        /* legacy stdio adds non-zero digits beyond needed precision */
	{ "%.0f", 340282366920938463463374607431768211456.0,
	         "340282366920938463463374607431768211456" },
#endif

	{ NULL, 0.0, NULL }
};

int test_snprintf(void)
{
	int i, j, k;
	int err=0;
	char b[2000];

	TEST(i, snprintf(0, 0, "%ld", 123456l), 6, "length returned %d != %d");
	TEST(i, snprintf(0, 0, "%.4s", "hello"), 4, "length returned %d != %d");
	TEST(i, snprintf(b, 0, "%.0s", "goodbye"), 0, "length returned %d != %d");

	strcpy(b, "xxxxxxxx");
	TEST(i, snprintf(b, 4, "%ld", 123456l), 6, "length returned %d != %d");
	TEST_S(b, "123", "incorrect output");
	TEST(i, b[5], 'x', "buffer overrun");

#if __SIZEOF_DOUBLE__ == 8
#if defined(TINY_STDIO) || (!defined(NO_FLOATING_POINT) && (!defined(_WANT_IO_LONG_DOUBLE) || defined(_LDBL_EQ_DBL)))
	/* Perform ascii arithmetic to test printing tiny doubles */
	TEST(i, snprintf(b, sizeof b, "%.1022f", 0x1p-1021), 1024, "%d != %d");
	b[1] = '0';
	for (i=0; i<1021; i++) {
		for (k=0, j=1023; j>0; j--) {
			if (b[j]<'5') b[j]+=b[j]-'0'+k, k=0;
			else b[j]+=b[j]-'0'-10+k, k=1;
		}
	}
	TEST(i, b[1], '1', "'%c' != '%c'");
// I have no idea what this is testing, but it's not sensible
//	for (j=2; b[j]=='0'; j++);
//	TEST(i, j, 1024, "%d != %d");
#endif
#endif


#ifndef DISABLE_SLOW_TESTS
	errno = 0;
	TEST(i, snprintf(NULL, 0, "%.*u", 2147483647, 0), 2147483647, "cannot print max length %d");
// picolibc doesn't check this condition
//	TEST(i, snprintf(NULL, 0, "%.*u ", 2147483647, 0), -1, "integer overflow %d");
//	TEST(i, errno, EOVERFLOW, "after overflow: %d != %d");
#endif
	for (j=0; int_tests[j].fmt; j++) {
                TEST(i, snprintf(b, sizeof b, int_tests[j].fmt, int_tests[j].i), (int) strlen(b), "%d != %d");
		TEST_S(b, int_tests[j].expect, "bad integer conversion");
	}

        (void) fp_tests;
        (void) k;
#if !defined(NO_FLOATING_POINT) && defined(_IO_FLOAT_EXACT) && __SIZEOF_DOUBLE__ == 8
	for (j=0; fp_tests[j].fmt; j++) {
		TEST(i, snprintf(b, sizeof b, fp_tests[j].fmt, fp_tests[j].f), (int) strlen(b), "%d != %d");
		TEST_S(b, fp_tests[j].expect, "bad floating point conversion");
	}

	TEST(i, snprintf(0, 0, "%.4a", 1.0), 11, "%d != %d");
#endif

	return err;
}

#define TEST_NAME snprintf
#include "testcase.h"
