/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
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

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <wchar.h>

#ifndef __PICOLIBC__
# define _WANT_IO_C99_FORMATS
# define _WANT_IO_LONG_LONG
# define _WANT_IO_POS_ARGS
# define printf_float(x) ((double) (x))
#elif defined(TINY_STDIO)
# ifdef PICOLIBC_INTEGER_PRINTF_SCANF
#  ifndef _WANT_IO_POS_ARGS
#   define NO_POS_ARGS
#  endif
# endif
# ifdef _WANT_IO_PERCENT_B
#  define BINARY_FORMAT
# endif
#else
#define printf_float(x) ((double) (x))

#ifndef _WANT_IO_POS_ARGS
#define NO_POS_ARGS
#endif

#ifdef _NANO_FORMATTED_IO

#ifndef NO_FLOATING_POINT
extern int _printf_float();
extern int _scanf_float();

int (*_reference_printf_float)() = _printf_float;
int (*_reference_scanf_float)() = _scanf_float;
#endif
#endif
#endif

#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
static const double test_vals[] = { 1.234567, 1.1, M_PI };
#endif

int
check_vsnprintf(char *str, size_t size, const char *format, ...)
{
	int i;
	va_list ap;

	va_start(ap, format);
	i = vsnprintf(str, size, format, ap);
	va_end(ap);
	return i;
}

#if defined(__PICOLIBC__) && !defined(TINYSTDIO)
#define LEGACY_NEWLIB
#endif

static struct {
    const wchar_t *str;
    const wchar_t *fmt;
    int expect;
} wtest[] = {
    { .str = L"foo\n", .fmt = L"foo\nbar", .expect = -1 },
    { .str = L"foo\n", .fmt = L"foo bar", .expect = -1 },
    { .str = L"foo\n", .fmt = L"foo %d", .expect = -1 },
    { .str = L"foo\n", .fmt = L"foo\n%d", .expect = -1 },
    { .str = L"foon", .fmt = L"foonbar", .expect = -1 },
    { .str = L"foon", .fmt = L"foon%d", .expect = -1 },
    { .str = L"foo ", .fmt = L"foo bar", .expect = -1 },
    { .str = L"foo ", .fmt = L"foo %d", .expect = -1 },
    { .str = L"foo\t", .fmt = L"foo\tbar", .expect = -1 },
    { .str = L"foo\t", .fmt = L"foo bar", .expect = -1 },
    { .str = L"foo\t", .fmt = L"foo %d", .expect = -1 },
    { .str = L"foo\t", .fmt = L"foo\t%d", .expect = -1 },
    { .str = L"foo", .fmt = L"foo", .expect = 0 },
#ifndef LEGACY_NEWLIB
    { .str = L"foon", .fmt = L"foo bar", .expect = 0 },
    { .str = L"foon", .fmt = L"foo %d", .expect = 0 },
    { .str = L"foo ", .fmt = L"fooxbar", .expect = 0 },
    { .str = L"foo ", .fmt = L"foox%d", .expect = 0 },
    { .str = L"foo bar", .fmt = L"foon", .expect = 0 },
#endif
    { .str = L"foo bar", .fmt = L"foo bar", .expect = 0 },
    { .str = L"foo bar", .fmt = L"foo %d", .expect = 0 },
#ifndef LEGACY_NEWLIB
    { .str = L"foo bar", .fmt = L"foon%d", .expect = 0 },
#endif
    { .str = L"foo (nil)", .fmt = L"foo %4p", .expect = 0},
    { .str = L"foo ", .fmt = L"foo %n", .expect = 0 },
    { .str = L"foo%bar1", .fmt = L"foo%%bar%d", 1 },
    { }
};

int
main(void)
{
	int x = -35;
	int y;
	char	buf[256];
	int	errors = 0;

#if 0
	double	a;

	printf ("hello world\n");
	for (x = 1; x < 20; x++) {
		printf("%.*f\n", x, 9.99999999999);
	}

	for (a = 1e-10; a < 1e10; a *= 10.0) {
		printf("g format: %10.3g %10.3g\n", 1.2345678 * a, 1.1 * a);
		fflush(stdout);
	}
	for (a = 1e-10; a < 1e10; a *= 10.0) {
		printf("f format: %10.3f %10.3f\n", 1.2345678 * a, 1.1 * a);
		fflush(stdout);
	}
	for (a = 1e-10; a < 1e10; a *= 10.0) {
		printf("e format: %10.3e %10.3e\n", 1.2345678 * a, 1.1 * a);
		fflush(stdout);
	}
	printf ("%g\n", exp(11));
#endif

        unsigned wt;
        for (wt = 0; wtest[wt].str; wt++) {
            void *extra;
            int wtr = swscanf(wtest[wt].str, wtest[wt].fmt, &extra);
            if (wtr != wtest[wt].expect) {
                wprintf(L"%d str %ls fmt %ls expected %d got %d\n", wt,
                        wtest[wt].str, wtest[wt].fmt, wtest[wt].expect, wtr);
                ++errors;
            }
        }
        wprintf(L"hello world %g\n", 1.0);

#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
	sprintf(buf, "%g", printf_float(0.0f));
	if (strcmp(buf, "0") != 0) {
		printf("0: wanted \"0\" got \"%s\"\n", buf);
		errors++;
		fflush(stdout);
	}
#endif

#ifndef NO_POS_ARGS
        x = y = 0;
        int r = sscanf("3 4", "%2$d %1$d", &x, &y);
        if (x != 4 || y != 3 || r != 2) {
            printf("pos: wanted %d %d (ret %d) got %d %d (ret %d)", 4, 3, 2, x, y, r);
            errors++;
            fflush(stdout);
        }
#endif

	/*
	 * test snprintf and vsnprintf to make sure they don't
	 * overwrite the specified buffer length (even if that is
	 * zero)
	 */
	for (x = 0; x <= 6; x++) {
		for (y = 0; y < 2; y++) {
			char tbuf[10] = "xxxxxxxxx";
			const char ref[10] = "xxxxxxxxx";
			const char *name = (y == 0 ? "snprintf" : "vsnprintf");
			int i = (y == 0 ? snprintf : check_vsnprintf) (tbuf, x, "%s", "123");
			int y = x <= 4 ? x : 4;
			if (i != 3) {
				printf("%s(tbuf, %d, \"%%s\", \"123\") return %d instead of %d\n", name,
				       x, i, 3);
				errors++;
			}
			int l = strlen(tbuf);
			if (y > 0 && l != y - 1) {
				printf("%s: returned buffer len want %d got %d\n", name, y - 1, l);
				errors++;
			}
			if (y > 0 && strncmp(tbuf, "123", y - 1) != 0) {
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wstringop-truncation"
				strncpy(buf, "123", y - 1);
				buf[y-1] = '\0';
				printf("%s: returned buffer want %s got %s\n", name, buf, tbuf);
				errors++;
			}
			if (memcmp(tbuf + y, ref + y, sizeof(tbuf) - y) != 0) {
				printf("%s: tail of buf mangled %s\n", name, tbuf + y);
				errors++;
			}
		}
	}

#define FMT(prefix,conv) "%" prefix conv

#define VERIFY_BOTH(prefix, oconv, iconv) do {                          \
            int __n;                                                    \
            sprintf(buf, FMT(prefix, oconv), v);                        \
            __n = sscanf(buf, FMT(prefix, iconv), &r);                  \
            if (v != r || __n != 1) {                                   \
                printf("\t%3d: " prefix " " oconv " wanted " FMT(prefix, oconv) " got " FMT(prefix, oconv) "\n", x, v, r); \
                errors++;                                               \
                fflush(stdout);                                         \
            }                                                           \
        } while(0)

#define VERIFY(prefix, conv) VERIFY_BOTH(prefix, conv, conv)

#ifdef BINARY_FORMAT
#define VERIFY_BINARY(prefix) VERIFY(prefix, "b")
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#else
#define VERIFY_BINARY(prefix)
#endif

#define CHECK_RT(type, prefix) do {                                     \
        for (x = 0; x < (int) (sizeof(type) * 8); x++) {                \
                type v = (type) (0x123456789abcdef0LL >> (64 - sizeof(type) * 8)) >> x; \
                type r = ~v;                                            \
                VERIFY(prefix, "d");                                    \
                r = ~v;                                                 \
                VERIFY(prefix, "u");                                    \
                r = ~v;                                                 \
                VERIFY(prefix, "x");                                    \
                r = ~v;                                                 \
                VERIFY(prefix, "o");                                    \
                r = ~v;                                                 \
                VERIFY_BINARY(prefix);                                  \
        }                                                               \
        } while(0)

#ifndef _NANO_FORMATTED_IO
	CHECK_RT(unsigned char, "hh");
#endif
	CHECK_RT(unsigned short, "h");
        CHECK_RT(unsigned int, "");
        CHECK_RT(unsigned long, "l");
#if defined(_WANT_IO_LONG_LONG) || defined(TINY_STDIO)
        CHECK_RT(unsigned long long, "ll");
#endif
#ifndef _NANO_FORMATTED_IO
#if !defined(_WANT_IO_LONG_LONG) && !defined(TINY_STDIO)
	if (sizeof(intmax_t) <= sizeof(long))
#endif
	{
	        CHECK_RT(intmax_t, "j");
	}
        CHECK_RT(size_t, "z");
        CHECK_RT(ptrdiff_t, "t");
#endif

        {
            static int i_addr = 12;
            void *v = &i_addr;
            void *r = (void *) -1;
            VERIFY("", "p");
        }
#if defined(TINY_STDIO) || !defined(NO_FLOATING_POINT)
#ifdef PICOLIBC_FLOAT_PRINTF_SCANF
#define float_type float
#define pow(a,b) powf((float) a, (float) b)
#define fabs(a) fabsf(a)
#define scanf_format "%f"
#if (defined(TINY_STDIO) && !defined(_IO_FLOAT_EXACT))
#define ERROR_MAX 1e-6
#else
#define ERROR_MAX 0
#endif
#else
#define float_type double
#define scanf_format "%lf"
#if defined(TINY_STDIO) && !defined(_IO_FLOAT_EXACT)
#define ERROR_MAX 1e-15
#else
#if (!defined(TINY_STDIO) && defined(_WANT_IO_LONG_DOUBLE))
/* __ldtoa is really broken */
#define ERROR_MAX 1e-5
#else
#define ERROR_MAX 0
#endif
#endif
#endif
	for (x = -37; x <= 37; x++)
	{
                float_type r;
		unsigned t;
		for (t = 0; t < sizeof(test_vals)/sizeof(test_vals[0]); t++) {

			float_type v = (float_type) test_vals[t] * pow(10.0, (float_type) x);
			float_type e;

			sprintf(buf, "%.55f", printf_float(v));
			sscanf(buf, scanf_format, &r);
			e = fabs(v-r) / v;
			if (e > (float_type) ERROR_MAX) {
				printf("\tf %3d: wanted %.7e got %.7e (error %.7e, buf %s)\n", x,
				       printf_float(v), printf_float(r), printf_float(e), buf);
				errors++;
				fflush(stdout);
			}


			sprintf(buf, "%.20e", printf_float(v));
			sscanf(buf, scanf_format, &r);
			e = fabs(v-r) / v;
			if (e > (float_type) ERROR_MAX)
			{
				printf("\te %3d: wanted %.7e got %.7e (error %.7e, buf %s)\n", x,
				       printf_float(v), printf_float(r), printf_float(e), buf);
				errors++;
				fflush(stdout);
			}


			sprintf(buf, "%.20g", printf_float(v));
			sscanf(buf, scanf_format, &r);
			e = fabs(v-r) / v;
			if (e > (float_type) ERROR_MAX)
			{
				printf("\tg %3d: wanted %.7e got %.7e (error %.7e, buf %s)\n", x,
				       printf_float(v), printf_float(r), printf_float(e), buf);
				errors++;
				fflush(stdout);
			}

#ifdef _WANT_IO_C99_FORMATS
			sprintf(buf, "%.20a", printf_float(v));
			sscanf(buf, scanf_format, &r);
			e = fabs(v-r) / v;
			if (e > (float_type) ERROR_MAX)
			{
				printf("\ta %3d: wanted %.7e got %.7e (error %.7e, buf %s)\n", x,
				       printf_float(v), printf_float(r), printf_float(e), buf);
				errors++;
				fflush(stdout);
			}
#endif

		}
#ifdef _WANT_IO_C99_FORMATS
                sprintf(buf, "0x0.0p%+d", x);
                sscanf(buf, scanf_format, &r);
                if (r != (float_type) 0.0)
                {
                    printf("\tg %3d: wanted 0.0 got %.7e (buf %s)\n", x,
                           printf_float(r), buf);
                    errors++;
                    fflush(stdout);
                }

                sprintf(buf, "0x1p%+d", x);
                sscanf(buf, scanf_format, &r);
                if (r != (float_type) ldexp(1.0, x))
                {
                    printf("\tg %3d: wanted 1 got %.7e (buf %s)\n", x,
                           printf_float(r), buf);
                    errors++;
                    fflush(stdout);
                }
#endif
	}
#endif
	fflush(stdout);
	return errors;
}
