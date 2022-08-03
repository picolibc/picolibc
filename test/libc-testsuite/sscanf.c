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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define TEST(r, f, x, m) ( \
((r) = (f)) == (x) || \
(printf(__FILE__ ":%d: %s failed (" m ")\n", __LINE__, #f, r, x), err++, 0) )

#define TEST_S(s, x, m) ( \
!strcmp((s),(x)) || \
(printf(__FILE__ ":%d: [%s] != [%s] (%s)\n", __LINE__, s, x, m), err++, 0) )

#define TEST_F(x) ( \
TEST(i, sscanf(# x, "%lf", &d), 1, "got %d fields, expected %d"), \
TEST(t, d, (double)x, "%g != %g") )

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wliteral-range"
#pragma GCC diagnostic ignored "-Wformat-extra-args"

int test_sscanf(void)
{
	int i;
	int err=0;
	char a[100], b[100];
	int x, y, z, u, v;
#ifdef _WANT_IO_PERCENT_B
        int w;
#endif
	double d, t;
//	long lo[10];

	TEST(i, sscanf("hello, world\n", "%s %s", a, b), 2, "only %d fields, expected %d");
	TEST_S(a, "hello,", "");
	TEST_S(b, "world", "");

	TEST(i, sscanf("hello, world\n", "%[hel]%s", a, b), 2, "only %d fields, expected %d");
	TEST_S(a, "hell", "");
	TEST_S(b, "o,", "");

	TEST(i, sscanf("hello, world\n", "%[hel] %s", a, b), 2, "only %d fields, expected %d");
	TEST_S(a, "hell", "");
	TEST_S(b, "o,", "");

#ifdef TINY_STDIO
	a[8] = 'X';
	a[9] = 0;
        /* legacy stdio fails this test */
	TEST(i, sscanf("hello, world\n", "%8c%8c", a, b), 1, "%d fields, expected %d");
	TEST_S(a, "hello, wX", "");
#endif

	TEST(i, sscanf("56789 0123 56a72", "%2d%d%*d %[0123456789]\n", &x, &y, a), 3, "only %d fields, expected %d");
	TEST(i, x, 56, "%d != %d");
	TEST(i, y, 789, "%d != %d");
	TEST_S(a, "56", "");

	TEST(i, sscanf("011 0x100 11 0x100 100", "%i %i %o %x %x\n", &x, &y, &z, &u, &v), 5, "only %d fields, expected %d");
	TEST(i, x, 9, "%d != %d");
	TEST(i, y, 256, "%d != %d");
	TEST(i, z, 9, "%d != %d");
	TEST(i, u, 256, "%d != %d");
	TEST(i, v, 256, "%d != %d");

#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"

#ifdef _WANT_IO_PERCENT_B
	TEST(i, sscanf("011 0x100 0b101 11 100 101", "%i %i %i %o %x %b\n", &x, &y, &z, &u, &v, &w), 6, "only %d fields, expected %d");
	TEST(i, x, 9, "%d != %d");
	TEST(i, y, 256, "%d != %d");
	TEST(i, z, 5, "%d != %d");
	TEST(i, u, 9, "%d != %d");
	TEST(i, v, 256, "%d != %d");
	TEST(i, w, 5, "%d != %d");
#endif

	TEST(i, sscanf("20 xyz", "%d %d\n", &x, &y), 1, "only %d fields, expected %d");
	TEST(i, x, 20, "%d != %d");

	TEST(i, sscanf("xyz", "%d\n", &x, &y), 0, "got %d fields, expected no match (%d)");

	TEST(i, sscanf("", "%d\n", &x, &y), -1, "got %d fields, expected input failure (%d)");

	TEST(i, sscanf(" 12345 6", "%2d%d%d", &x, &y, &z), 3, "only %d fields, expected %d");
	TEST(i, x, 12, "%d != %d");
	TEST(i, y, 345, "%d != %d");
	TEST(i, z, 6, "%d != %d");

// tinystdio only has one ungetc spot, so it can't unparse numbers
//	TEST(i, sscanf(" 0x12 0x34", "%5i%2i", &x, &y), 1, "got %d fields, expected %d");
//	TEST(i, x, 0x12, "%d != %d");

        (void) d;
        (void) t;
#if !defined(NO_FLOATING_POINT) && defined(_IO_FLOAT_EXACT)
	TEST_F(123);
	TEST_F(123.0);
	TEST_F(123.0e+0);
	TEST_F(123.0e+4);
	TEST_F(1.234e1234);
	TEST_F(1.234e-1234);
	TEST_F(1.234e56789);
	TEST_F(1.234e-56789);
	TEST_F(-0.5);
	TEST_F(0.1);
	TEST_F(0.2);
	TEST_F(0.1e-10);
	TEST_F(0x1234p56);

#ifndef __PICOLIBC__
        /* both tinystdio and legacy stdio fail this test */
	TEST(i, sscanf("10e", "%lf", &d), 0, "got %d fields, expected no match (%d)");
#endif
	TEST(i, sscanf("", "%lf\n", &d), -1, "got %d fields, expected input failure (%d)");
#endif


	return err;
}

#define TEST_NAME sscanf
#include "testcase.h"
