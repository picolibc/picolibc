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
#define _BSD_SOURCE
#include <stdio.h>
#include <string.h>

/* r = place to store result
 * f = function call to test (or any expression)
 * x = expected result
 * m = message to print on failure (with formats for r & x)
**/

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wstringop-truncation"

#define TEST(r, f, x, m) do {                                           \
        (r) = (f);                                                      \
        if ((r) != (x)) {                                               \
            printf("%s:%d: %s failed (" m ")\n", __FILE__, __LINE__, #f, r, x ); \
            err++;                                                      \
        }                                                               \
    } while(0)

#define TEST_S(s, x, m) do {                                            \
        if (strcmp((s),(x)) != 0) {                                     \
            printf(__FILE__ ":%d: [%s] != [%s] (%s)\n", __LINE__, s, x, m); \
            err++;                                                      \
        }                                                               \
    } while(0)


int test_string(void)
{
	char b[32];
	char *s;
	int i;
	int err=0;

	b[16]='a'; b[17]='b'; b[18]='c'; b[19]=0;
	TEST(s, strcpy(b, b+16), b, "wrong return %p != %p");
	TEST_S(s, "abc", "strcpy gave incorrect string");
	TEST(s, strcpy(b+1, b+16), b+1, "wrong return %p != %p");
	TEST_S(s, "abc", "strcpy gave incorrect string");
	TEST(s, strcpy(b+2, b+16), b+2, "wrong return %p != %p");
	TEST_S(s, "abc", "strcpy gave incorrect string");
	TEST(s, strcpy(b+3, b+16), b+3, "wrong return %p != %p");
	TEST_S(s, "abc", "strcpy gave incorrect string");

	TEST(s, strcpy(b+1, b+17), b+1, "wrong return %p != %p");
	TEST_S(s, "bc", "strcpy gave incorrect string");
	TEST(s, strcpy(b+2, b+18), b+2, "wrong return %p != %p");
	TEST_S(s, "c", "strcpy gave incorrect string");
	TEST(s, strcpy(b+3, b+19), b+3, "wrong return %p != %p");
	TEST_S(s, "", "strcpy gave incorrect string");

	TEST(s, memset(b, 'x', sizeof b), b, "wrong return %p != %p");
	TEST(s, strncpy(b, "abc", sizeof b - 1), b, "wrong return %p != %p");
	TEST(i, memcmp(b, "abc\0\0\0\0", 8), 0, "strncpy fails to zero-pad dest");
	TEST(i, b[sizeof b - 1], 'x', "strncpy overruns buffer when n > strlen(src)");

	b[3] = 'x'; b[4] = 0;
	strncpy(b, "abc", 3);
	TEST(i, b[2], 'c', "strncpy fails to copy last byte: %hhu != %hhu");
	TEST(i, b[3], 'x', "strncpy overruns buffer to null-terminate: %hhu != %hhu");

	TEST(i, !strncmp("abcd", "abce", 3), 1, "strncmp compares past n");
	TEST(i, !!strncmp("abc", "abd", 3), 1, "strncmp fails to compare n-1st byte");

	strcpy(b, "abc");
	TEST(s, strncat(b, "123456", 3), b, "%p != %p");
	TEST(i, b[6], 0, "strncat failed to null-terminate (%d)");
	TEST_S(s, "abc123", "strncat gave incorrect string");

	strcpy(b, "aaababccdd0001122223");
	TEST(s, strchr(b, 'b'), b+3, "%p != %p");
	TEST(s, strrchr(b, 'b'), b+5, "%p != %p");
	TEST(i, strspn(b, "abcd"), 10, "%d != %d");
	TEST(i, strcspn(b, "0123"), 10, "%d != %d");
	TEST(s, strpbrk(b, "0123"), b+10, "%p != %p");

	strcpy(b, "abc   123; xyz; foo");
	TEST(s, strtok(b, " "), b, "%p != %p");
	TEST_S(s, "abc", "strtok result");

	TEST(s, strtok(NULL, ";"), b+4, "%p != %p");
	TEST_S(s, "  123", "strtok result");

	TEST(s, strtok(NULL, " ;"), b+11, "%p != %p");
	TEST_S(s, "xyz", "strtok result");

	TEST(s, strtok(NULL, " ;"), b+16, "%p != %p");
	TEST_S(s, "foo", "strtok result");

#ifdef HAVE_BSD_STRL
	memset(b, 'x', sizeof b);
	TEST(i, strlcpy(b, "abc", sizeof b - 1), 3, "length %d != %d");
	TEST(i, b[3], 0, "strlcpy did not null-terminate short string (%d)");
	TEST(i, b[4], 'x', "strlcpy wrote extra bytes (%d)");

	memset(b, 'x', sizeof b);
	TEST(i, strlcpy(b, "abc", 2), 3, "length %d != %d");
	TEST(i, b[0], 'a', "strlcpy did not copy character %d");
	TEST(i, b[1], 0, "strlcpy did not null-terminate long string (%d)");

	memset(b, 'x', sizeof b);
	TEST(i, strlcpy(b, "abc", 3), 3, "length %d != %d");
	TEST(i, b[2], 0, "strlcpy did not null-terminate l-length string (%d)");

	TEST(i, strlcpy(NULL, "abc", 0), 3, "length %d != %d");

	memcpy(b, "abc\0\0\0x\0", 8);
	TEST(i, strlcat(b, "123", sizeof b), 6, "length %d != %d");
	TEST_S(b, "abc123", "strlcat result");

	memcpy(b, "abc\0\0\0x\0", 8);
	TEST(i, strlcat(b, "123", 6), 6, "length %d != %d");
	TEST_S(b, "abc12", "strlcat result");
	TEST(i, b[6], 'x', "strlcat wrote past string %d != %d");

	memcpy(b, "abc\0\0\0x\0", 8);
	TEST(i, strlcat(b, "123", 4), 6, "length %d != %d");
	TEST_S(b, "abc", "strlcat result");

	memcpy(b, "abc\0\0\0x\0", 8);
	TEST(i, strlcat(b, "123", 3), 6, "length %d != %d");
	TEST_S(b, "abc", "strlcat result");
#endif

	return err;
}

#define TEST_NAME string
#include "testcase.h"
