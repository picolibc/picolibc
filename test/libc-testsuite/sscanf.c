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
#include <locale.h>
#include <wchar.h>
#include <math.h>

#define TEST(r, f, x, m)                                                             \
    (((r) = (f)) == (x)                                                              \
     || (printf(__FILE__ ":%d: %s failed (" m ")\n", __LINE__, #f, r, x), err++, 0))

#define TEST_S(s, x, m)                                                             \
    (!strcmp((s), (x))                                                              \
     || (printf(__FILE__ ":%d: [%s] != [%s] (%s)\n", __LINE__, s, x, m), err++, 0))

#define TEST_WS(s, x, m)                                                               \
    (!wcscmp((s), (x))                                                                 \
     || (printf(__FILE__ ":%d: [%ls] != [%ls] (%ls)\n", __LINE__, s, x, m), err++, 0))

#define TEST_F(x)                                                     \
    (TEST(i, sscanf(#x, "%lf", &d), 1, "got %d fields, expected %d"), \
     TEST(t, d, (double)x, "%a != %a"))

#define TEST_FV(x, v)                                                \
    (TEST(i, sscanf(v, "%lf", &d), 1, "got %d fields, expected %d"), \
     TEST(t, d, (double)x, "%a != %a"))

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wliteral-range"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif

#if defined(__PICOLIBC__) && !defined(__MB_CAPABLE)
#define NO_MULTI_BYTE
#endif

#ifdef __RX__
#define NO_NAN
#define NO_INF
#endif

static int
test_sscanf(void)
{
    int     i;
    int     err = 0;
    char    a[100], b[100], c[100];
    wchar_t aw[100], bw[100], cw[100];
    int     x, y, z, u, v;
#ifdef __IO_PERCENT_B
    int w;
#endif
    double d, t;
    //	long lo[10];

#if !defined(__PICOLIBC__) || defined(__MB_CAPABLE)
    if (!setlocale(LC_CTYPE, "C.UTF-8")) {
        printf("setlocale(LC_CTYPE, \"C.UTF-8\") failed\n");
        return 1;
    }
#endif
    (void)aw;
    (void)bw;
    (void)cw;

    /* sscanf with char results */

    TEST(i, sscanf("hello, world\n", "%s %s", a, b), 2, "only %d fields, expected %d");
    TEST_S(a, "hello,", "");
    TEST_S(b, "world", "");

    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));
    memset(c, 0, sizeof(c));
    TEST(i, sscanf("hello, world\n", "%6c %5c%c", a, b, c), 3, "only %d fields, expected %d");
    TEST_S(a, "hello,", "");
    TEST_S(b, "world", "");
    TEST_S(c, "\n", "");

    TEST(i, sscanf("hello, world\n", "%[hel]%s", a, b), 2, "only %d fields, expected %d");
    TEST_S(a, "hell", "");
    TEST_S(b, "o,", "");

    TEST(i, sscanf("hello, world\n", "%[hel] %s", a, b), 2, "only %d fields, expected %d");
    TEST_S(a, "hell", "");
    TEST_S(b, "o,", "");

    TEST(i, sscanf("elloworld", "%1[abcdefg]%s", a, b), 2, "only %d fields, expected %d");
    TEST_S(a, "e", "");
    TEST_S(b, "lloworld", "");

    TEST(i, sscanf("elloworld", "%5[a-z]%s", a, b), 2, "only %d fields, expected %d");
    TEST_S(a, "ellow", "");
    TEST_S(b, "orld", "");

    memset(a, 0, sizeof(a));
    TEST(i, sscanf("", "%c", a), -1, "only %d fields, expected %d");
    TEST_S(a, "", "");

    memset(a, 0, sizeof(a));
    TEST(i, sscanf("a", "%2c", a), 1, "only %d fields, expected %d");
    TEST_S(a, "a", "");

#ifndef NO_MULTI_BYTE
    /* sscanf with mb results */

    TEST(i, sscanf("㌰ello, ✕orld\n", "%s %s", a, b), 2, "only %d fields, expected %d");
    TEST_S(a, "㌰ello,", "");
    TEST_S(b, "✕orld", "");

    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));
    memset(c, 0, sizeof(c));
    TEST(i, sscanf("㌰ello, ✕orld\n", "%8c %7c%c", a, b, c), 3, "only %d fields, expected %d");
    TEST_S(a, "㌰ello,", "");
    TEST_S(b, "✕orld", "");
    TEST_S(c, "\n", "");

    TEST(i, sscanf("㌰ello, ✕orld\n", "%[㌰el]%s", a, b), 2, "only %d fields, expected %d");
    TEST_S(a, "㌰ell", "");
    TEST_S(b, "o,", "");

    TEST(i, sscanf("㌰ello, ✕orld\n", "%[㌰el] %s", a, b), 2, "only %d fields, expected %d");
    TEST_S(a, "㌰ell", "");
    TEST_S(b, "o,", "");

    /* sscanf with wchar results */

    TEST(i, sscanf("㌰ello, ✕orld\n", "%ls %ls", aw, bw), 2, "only %d fields, expected %d");
    TEST_WS(aw, L"㌰ello,", L"");
    TEST_WS(bw, L"✕orld", L"");

    memset(aw, 0, sizeof(aw));
    memset(bw, 0, sizeof(bw));
    memset(cw, 0, sizeof(cw));
    TEST(i, sscanf("㌰ello, ✕orld\n", "%6lc %5lc%lc", aw, bw, cw), 3,
         "only %d fields, expected %d");
    TEST_WS(aw, L"㌰ello,", L"");
    TEST_WS(bw, L"✕orld", L"");
    TEST_WS(cw, L"\n", L"");

    TEST(i, sscanf("㌰ello, ✕orld\n", "%l[^o]%ls", aw, bw), 2, "only %d fields, expected %d");
    TEST_WS(aw, L"㌰ell", L"");
    TEST_WS(bw, L"o,", L"");

    TEST(i, sscanf("㌰ello, ✕orld\n", "%l[^o] %ls", aw, bw), 2, "only %d fields, expected %d");
    TEST_S(a, "㌰ell", "");
    TEST_S(b, "o,", "");

    /* swscanf with mb results */

    TEST(i, swscanf(L"hello, world\n", L"%s %s", a, b), 2, "only %d fields, expected %d");
    TEST_S(a, "hello,", "");
    TEST_S(b, "world", "");

    TEST(i, swscanf(L"㌰ello, ✕orld\n", L"%s %s", a, b), 2, "only %d fields, expected %d");
    TEST_S(a, "㌰ello,", "");
    TEST_S(b, "✕orld", "");

    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));
    memset(c, 0, sizeof(c));
    TEST(i, swscanf(L"hello, world\n", L"%6c %5c%c", a, b, c), 3, "only %d fields, expected %d");
    TEST_S(a, "hello,", "");
    TEST_S(b, "world", "");
    TEST_S(c, "\n", "");

    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));
    memset(c, 0, sizeof(c));
    TEST(i, swscanf(L"㌰ello, ✕orld\n", L"%6c %5c%c", a, b, c), 3, "only %d fields, expected %d");
    TEST_S(a, "㌰ello,", "");
    TEST_S(b, "✕orld", "");
    TEST_S(c, "\n", "");

    /* swscanf with wchar results */

    TEST(i, swscanf(L"㌰ello, ✕orld\n", L"%ls %ls", aw, bw), 2, "only %d fields, expected %d");
    TEST_WS(aw, L"㌰ello,", L"");
    TEST_WS(bw, L"✕orld", L"");

    memset(aw, 0, sizeof(aw));
    memset(bw, 0, sizeof(bw));
    memset(cw, 0, sizeof(cw));
    TEST(i, swscanf(L"㌰ello, ✕orld\n", L"%6lc %5lc%lc", aw, bw, cw), 3,
         "only %d fields, expected %d");
    TEST_WS(aw, L"㌰ello,", L"");
    TEST_WS(bw, L"✕orld", L"");
    TEST_WS(cw, L"\n", L"");
#endif

    a[8] = 'X';
    a[9] = 0;
    memset(b, 0, sizeof(b));
    TEST(i, sscanf("hello, world\n", "%8c%8c", a, b), 2, "%d fields, expected %d");
    TEST_S(a, "hello, wX", "");
    TEST_S(b, "orld\n", "");

#ifndef NO_NAN
    /* testing nan(n-seq-char) in the expected form */
    a[0] = '#';
    a[1] = '\0';
    TEST(i, sscanf(" Nan(98b)", "%lf%c", &d, a), 1, "got %d fields, expected %d");
    TEST(i, isnan(d), 1, "isnan %d expected %d");
    TEST_S(a, "#", "");

    /* testing nan(n-seq-char) missing closing paren */
    a[0] = '#';
    a[1] = '\0';
    d = 1.0;
#ifdef __PICOLIBC__
#define NAN_INCOMPLETE_RET -1
#else
/* glibc has a bug -- it returns 0 when reaching EOF without any conversions */
#define NAN_INCOMPLETE_RET 0
#endif
    TEST(i, sscanf("NaN(abcdefg", "%lf%c", &d, a), NAN_INCOMPLETE_RET,
         "got %d fields, expected %d");
    TEST(i, d, 1.0, "%d expected %lf");
    TEST_S(a, "#", "");

    /* testing nan(n-seq-char) invalid character inside parens */
    a[0] = '#';
    a[1] = '\0';
    d = 1.0;
    TEST(i, sscanf("NaN(12:b)", "%lf%c", &d, a), 0, "got %d fields, expected %d");
    TEST(i, d, 1.0, "%d expected %lf");
    TEST_S(a, "#", "");

    /* testing nan(n-seq-char) overrunning a field width value */
    a[0] = '#';
    a[1] = '\0';
    d = 1.0;
    TEST(i, sscanf("Nan(12345)", "%9lf%c", &d, a), 0, "got %d fields, expected %d");
    TEST(i, d, 1.0, "%d expected %lf");
    TEST_S(a, "#", "");
#endif

#ifndef NO_INF
    /* testing inf(n-seq-char) should 'inf' should be evaluated sperately */
    a[0] = '#';
    a[1] = '\0';
    d = 1.0;
    TEST(i, sscanf("inf(12b)", "%lf%c", &d, a), 2, "got %d fields, expected %d");
    TEST(i, isinf(d), 1, "%d expected %d");
    TEST_S(a, "(", "");

    /* testing infinity(n-seq-char) should 'infinity' should be evaluated sperately */
    a[0] = '#';
    a[1] = '\0';
    d = 1.0;
    TEST(i, sscanf("infinity(abcd)", "%lf%c", &d, a), 2, "got %d fields, expected %d");
    TEST(i, isinf(d), 1, "%d expected %d");
    TEST_S(a, "(", "");
#endif

    TEST(i, sscanf("56789 0123 56a72", "%2d%d%*d %[0123456789]\n", &x, &y, a), 3,
         "only %d fields, expected %d");
    TEST(i, x, 56, "%d != %d");
    TEST(i, y, 789, "%d != %d");
    TEST_S(a, "56", "");

    TEST(i, sscanf("011 0x100 11 0x100 100", "%i %i %o %x %x\n", &x, &y, &z, &u, &v), 5,
         "only %d fields, expected %d");
    TEST(i, x, 9, "%d != %d");
    TEST(i, y, 256, "%d != %d");
    TEST(i, z, 9, "%d != %d");
    TEST(i, u, 256, "%d != %d");
    TEST(i, v, 256, "%d != %d");

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif

#ifdef __IO_PERCENT_B
    TEST(i, sscanf("011 0x100 0b101 11 100 101", "%i %i %i %o %x %b\n", &x, &y, &z, &u, &v, &w), 6,
         "only %d fields, expected %d");
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

    // stdio only has one ungetc spot, so it can't unparse numbers
    //	TEST(i, sscanf(" 0x12 0x34", "%5i%2i", &x, &y), 1, "got %d fields, expected %d");
    //	TEST(i, x, 0x12, "%d != %d");

    (void)d;
    (void)t;
#if !defined(__IO_NO_FLOATING_POINT) && defined(__IO_FLOAT_EXACT)
    TEST_F(123);
    TEST_F(123.0);
    TEST_F(123.0e+0);
    TEST_F(123.0e+4);
#ifndef NO_INF
    TEST_F(1.234e1234);
    TEST_F(1.234e-1234);
    TEST_F(1.234e56789);
    TEST_F(1.234e-56789);
#endif
    TEST_F(-0.5);
    TEST_F(0.1);
    TEST_F(0.2);
    TEST_F(0.1e-10);
    TEST_F(0x1234p56);
    TEST_F(3752432815e-39);
#ifndef NO_NAN
    TEST(i, sscanf("nan", "%lg", &d), 1, "got %d fields, expected %d");
    TEST(i, isnan(d), 1, "isnan %d expected %d");
    TEST(i, !!signbit(d), 0, "signbit %d expected %d");
    TEST(i, sscanf("-nan", "%lg", &d), 1, "got %d fields, expected %d");
    TEST(i, isnan(d), 1, "isnan %d expected %d");
    TEST(i, !!signbit(d), 1, "signbit %d expected %d");
#endif
#ifndef NO_INF
    TEST_FV(INFINITY, "inf");
    TEST_FV(-INFINITY, "-inf");
#endif

#ifndef NO_INF
    d = 1.0;
    TEST(i, sscanf("-inf", "%3lg", &d), 0, "got %d fields, expected %d");
    TEST(i, d, 1.0, "%g expected %g");

    TEST(i, sscanf("-inf", "%4lg", &d), 1, "got %d fields, expected %d");
    TEST(i, isinf(d), 1, "isinf %d expected %d");
    TEST(i, !!signbit(d), 1, "signbit %d expected %d");
#endif

#ifndef __PICOLIBC__
    /* picolibc fails this test */
    TEST(i, sscanf("10e", "%lf", &d), 0, "got %d fields, expected no match (%d)");
#endif
    TEST(i, sscanf("", "%lf\n", &d), -1, "got %d fields, expected input failure (%d)");
#endif

    return err;
}

#define TEST_NAME sscanf
#include "testcase.h"
