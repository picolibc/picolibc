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

/* r = place to store result
 * f = function call to test (or any expression)
 * x = expected result
 * m = message to print on failure (with formats for r & x)
**/

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#pragma GCC diagnostic ignored "-Wformat"

#define TEST(r, f, x, m) ( \
msg = #f, ((r) = (f)) == (x) || \
(printf(__FILE__ ":%d: %s failed (" m ")\n", __LINE__, #f, r, x), err++, 0) )

#define TEST2(r, f, x, m) ( \
((r) = (f)) == (x) || \
(printf(__FILE__ ":%d: %s failed (" m ")\n", __LINE__, msg, r, x), err++, 0) )

int test_strtol(void)
{
	int i;
	long l;
	unsigned long ul;
	long long ll;
	unsigned long long ull;
	char *msg="";
	int err=0;
	char *s, *c;

        (void) ll;
        (void) ull;
	TEST(l, atol("2147483647"), 2147483647L, "max 32bit signed %ld != %ld");
	
	TEST(l, strtol("2147483647", 0, 0), 2147483647L, "max 32bit signed %ld != %ld");
	TEST(ul, strtoul("4294967295", 0, 0), 4294967295UL, "max 32bit unsigned %lu != %lu");

	if (sizeof(long) == 4) {
		errno = 0;
		TEST(l, strtol(s="2147483648", &c, 0), 2147483647L, "uncaught overflow %ld != %ld");
		TEST2(i, c-s, 10, "wrong final position %d != %d");
		TEST2(i, errno, ERANGE, "missing errno %d != %d");
		errno = 0;
		TEST(l, strtol(s="-2147483649", &c, 0), -2147483647L-1, "uncaught overflow %ld != %ld");
		TEST2(i, c-s, 11, "wrong final position %d != %d");
		TEST2(i, errno, ERANGE, "missing errno %d != %d");
		errno = 0;
		TEST(ul, strtoul(s="4294967296", &c, 0), 4294967295UL, "uncaught overflow %lu != %lu");
		TEST2(i, c-s, 10, "wrong final position %d != %d");
		TEST2(i, errno, ERANGE, "missing errno %d != %d");
		errno = 0;
		TEST(ul, strtoul(s="-1", &c, 0), -1UL, "rejected negative %lu != %lu");
		TEST2(i, c-s, 2, "wrong final position %d != %d");
		TEST2(i, errno, 0, "spurious errno %d != %d");
		errno = 0;
		TEST(ul, strtoul(s="-2", &c, 0), -2UL, "rejected negative %lu != %lu");
		TEST2(i, c-s, 2, "wrong final position %d != %d");
		TEST2(i, errno, 0, "spurious errno %d != %d");
		errno = 0;
		TEST(ul, strtoul(s="-2147483648", &c, 0), -2147483648UL, "rejected negative %lu != %lu");
		TEST2(i, c-s, 11, "wrong final position %d != %d");
		TEST2(i, errno, 0, "spurious errno %d != %d");
		errno = 0;
		TEST(ul, strtoul(s="-2147483649", &c, 0), -2147483649UL, "rejected negative %lu != %lu");
		TEST2(i, c-s, 11, "wrong final position %d != %d");
		TEST2(i, errno, 0, "spurious errno %d != %d");
	} else {
		errno = 0;
		TEST(l, strtol(s="9223372036854775808", &c, 0), 9223372036854775807L, "uncaught overflow %ld != %ld");
		TEST2(i, c-s, 19, "wrong final position %d != %d");
		TEST2(i, errno, ERANGE, "missing errno %d != %d");
		errno = 0;
		TEST(l, strtol(s="-9223372036854775809", &c, 0), -9223372036854775807L-1, "uncaught overflow %ld != %ld");
		TEST2(i, c-s, 20, "wrong final position %d != %d");
		TEST2(i, errno, ERANGE, "missing errno %d != %d");
		errno = 0;
		TEST(ul, strtoul(s="18446744073709551616", &c, 0), 18446744073709551615UL, "uncaught overflow %lu != %lu");
		TEST2(i, c-s, 20, "wrong final position %d != %d");
		TEST2(i, errno, ERANGE, "missing errno %d != %d");
		errno = 0;
		TEST(ul, strtoul(s="-1", &c, 0), -1UL, "rejected negative %lu != %lu");
		TEST2(i, c-s, 2, "wrong final position %d != %d");
		TEST2(i, errno, 0, "spurious errno %d != %d");
		errno = 0;
		TEST(ul, strtoul(s="-2", &c, 0), -2UL, "rejected negative %lu != %lu");
		TEST2(i, c-s, 2, "wrong final position %d != %d");
		TEST2(i, errno, 0, "spurious errno %d != %d");
		errno = 0;
		TEST(ul, strtoul(s="-9223372036854775808", &c, 0), -9223372036854775808UL, "rejected negative %lu != %lu");
		TEST2(i, c-s, 20, "wrong final position %d != %d");
		TEST2(i, errno, 0, "spurious errno %d != %d");
		errno = 0;
		TEST(ul, strtoul(s="-9223372036854775809", &c, 0), -9223372036854775809UL, "rejected negative %lu != %lu");
		TEST2(i, c-s, 20, "wrong final position %d != %d");
		TEST2(i, errno, 0, "spurious errno %d != %d");
	}

	TEST(l, strtol("z", 0, 36), 35, "%ld != %ld");
	TEST(l, strtol("00010010001101000101011001111000", 0, 2), 0x12345678, "%ld != %ld");
	TEST(l, strtol(s="0F5F", &c, 16), 0x0f5f, "%ld != %ld");

	TEST(l, strtol(s="0xz", &c, 16), 0, "%ld != %ld");
	TEST2(i, c-s, 1, "wrong final position %ld != %ld");

	TEST(l, strtol(s="0x1234", &c, 16), 0x1234, "%ld != %ld");
	TEST2(i, c-s, 6, "wrong final position %ld != %ld");

        char delim_buf[6] = "09af:";

        for (int j = 0; j < 256; j++) {
            delim_buf[4] = j;
            if (('0' <= j && j <= '9') ||
                ('A' <= j && j <= 'F') ||
                ('a' <= j && j <= 'f'))
            {
                int k;
                if ('0' <= j && j <= '9')
                    k = j - '0';
                else if ('A' <= j && j <= 'Z')
                    k = j - 'A' + 10;
                else if ('a' <= j && j <= 'z')
                    k = j - 'a' + 10;
                else
                    k = 0xffffffff;
                TEST(l, strtol(s=delim_buf, &c, 16), 0x09af0 | k, "%ld != %ld");
                TEST2(i, c-s, 5, "wrong final position %ld != %ld");
            } else {
                TEST(l, strtol(s=delim_buf, &c, 16), 0x09af, "%ld != %ld");
                TEST2(i, c-s, 4, "wrong final position %ld != %ld");
            }
        }

	errno = 0;
	c = NULL;
	TEST(l, strtol(s="123", &c, 37), 0, "%ld != %ld");
	TEST2(i, c-s, 0, "wrong final position %d != %d");
	TEST2(i, errno, EINVAL, "%d != %d");

	TEST(l, strtol(s="  15437", &c, 8), 015437, "%ld != %ld");
	TEST2(i, c-s, 7, "wrong final position %d != %d");

	TEST(l, strtol(s="  1", &c, 0), 1, "%ld != %ld");
	TEST2(i, c-s, 3, "wrong final position %d != %d");

	return err;
}

#define TEST_NAME strtol
#include "testcase.h"
