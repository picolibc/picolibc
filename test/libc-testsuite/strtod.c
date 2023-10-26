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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

/* r = place to store result
 * f = function call to test (or any expression)
 * x = expected result
 * m = message to print on failure (with formats for r & x)
**/

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wformat-extra-args"

#define TEST(r, f, x, m) ( \
msg = #f, ((r) = (f)) == (x) || \
(printf(__FILE__ ":%d: %s failed (" m ")\n", __LINE__, #f, r, x, r-x), err++, 0) )

#define TEST2(r, f, x, m) ( \
((r) = (f)) == (x) || \
(printf(__FILE__ ":%d: %s failed (" m ")\n", __LINE__, msg, r, x), err++, 0) )

int test_strtod(void)
{
	int i;
	double d, d2;
	char buf[1000];
	char *msg="";
	int err=0;

        (void) msg;
	for (i=0; i<100; i++) {
		d = sin(i);
		snprintf(buf, sizeof buf, "%.300f", d);
		TEST(d2, strtod(buf, 0), d, "round trip fail %a != %a (%a)");
	}

#if defined(__PICOLIBC__) && !defined(TINY_STDIO) && __SIZEOF_DOUBLE__ != 8
        /* skip strtod tests for legacy stdio on systems with atypical doubles */
#else
	TEST(d, strtod("0x1p4", 0), 16.0, "hex float %a != %a");
	TEST(d, strtod("0x1.1p4", 0), 17.0, "hex float %a != %a");
#endif

	return err;
}

#define TEST_NAME strtod
#include "testcase.h"
