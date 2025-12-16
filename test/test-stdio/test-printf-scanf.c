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
#include <locale.h>
#include <limits.h>

#ifndef __PICOLIBC__
# define printf_float(x) ((double) (x))
#else
# if !defined(_HAS_IO_FLOAT) && !defined(_HAS_IO_DOUBLE)
#  define __IO_NO_FLOATING_POINT
# endif
# ifndef _HAS_IO_POS_ARGS
#  define NO_POS_ARGS
# endif
# ifndef _HAS_IO_MBCHAR
#  define NO_MULTI_BYTE
# endif
# ifndef _HAS_IO_LONG_LONG
#  define NO_LONG_LONG
# endif
# ifndef _HAS_IO_C99_FORMATS
#  define NO_C99_FORMATS
# endif
# ifdef _HAS_IO_PERCENT_B
#  define BINARY_FORMAT
# endif
#endif

#if !defined(__IO_NO_FLOATING_POINT)
static const double test_vals[] = { 1.234567, 1.1, M_PI };
#endif

static int
check_vsnprintf(char *str, size_t size, const char *format, ...)
{
	int i;
	va_list ap;

	va_start(ap, format);
	i = vsnprintf(str, size, format, ap);
	va_end(ap);
	return i;
}

#if !defined(__IO_NO_FLOATING_POINT)
#if _PICOLIBC_PRINTF == 'f'
#define float_type float
#define pow(a,b) powf((float) (a), (float) (b))
#define nextafter(a,b) nextafterf((float)(a), (float)(b))
#define fabs(a) fabsf(a)
#define scanf_format "%f"
#if (defined(__PICOLIBC__) && !defined(__IO_FLOAT_EXACT))
#define ERROR_MAX 1e-6
#else
#define ERROR_MAX 0
#endif
#else
#define float_type double
#define scanf_format "%lf"
#if defined(__PICOLIBC__) && !defined(__IO_FLOAT_EXACT)
# if __SIZEOF_DOUBLE__ == 4
#  define ERROR_MAX 1e-6
# else
#  define ERROR_MAX 1e-14
# endif
#else
#define ERROR_MAX 0
#endif
#endif
#endif

#ifndef NO_WIDE_IO
static const struct {
    const wchar_t *const str;
    const wchar_t *const fmt;
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
    { .str = L"foon", .fmt = L"foo bar", .expect = 0 },
    { .str = L"foon", .fmt = L"foo %d", .expect = 0 },
    { .str = L"foo ", .fmt = L"fooxbar", .expect = 0 },
    { .str = L"foo ", .fmt = L"foox%d", .expect = 0 },
    { .str = L"foo bar", .fmt = L"foon", .expect = 0 },
    { .str = L"foo bar", .fmt = L"foo bar", .expect = 0 },
    { .str = L"foo bar", .fmt = L"foo %d", .expect = 0 },
    { .str = L"foo bar", .fmt = L"foon%d", .expect = 0 },
    { .str = L"foo (nil)", .fmt = L"foo %4p", .expect = 0},
    { .str = L"foo ", .fmt = L"foo %n", .expect = 0 },
    { .str = L"foo%bar1", .fmt = L"foo%%bar%d", 1 },
    { }
};
#endif

#ifndef __IO_NO_FLOATING_POINT

static float_type
int_exp10(int n)
{
        float_type      a = 1.0;
        int             sign = n < 0;

        if (sign)
                n = -n;
        while (n--)
                a *= (float_type) 10.0;
        if (sign)
                a = 1/a;
        return a;
}

#ifndef NO_FLOAT_EXACT

static int
check_float(const char *test_name, float_type a, const char *buf, const char *head, int zeros, const char *tail)
{
        int z;
        int o;

        if (strncmp(buf, head, strlen(head)) != 0)
                goto fail;

        o = strlen(head);
        for (z = 0; z < zeros; z++) {
                if (buf[o] != '0')
                        goto fail;
                o++;
        }

        if (strcmp(buf + o, tail) != 0)
                goto fail;
        return 0;

fail:
        printf("float mismatch test %s: %a %.17e \"%s\" isn't in the form \"%s\", %d zeros, \"%s\"\n",
               test_name, printf_float(a), printf_float(a), buf, head, zeros, tail);
        return 1;
}

#endif
#endif

/*
 * Compute arithmetic right shift. For negative values, flip the bits,
 * shift and flip back. Otherwise, just shift. Thanks to Per Vognsen
 * for this version which both gcc and clang both currently optimize
 * to a single asr instruction in small tests.
 */
#define __asr(t, n)                             \
    static inline t                             \
    __asr_ ## n(t x, int s) {                   \
        return x < 0 ? ~(~x >> s) : x >> s;     \
    }                                           \

__asr(signed char, signed_char)
__asr(short, short)
__asr(int, int)
__asr(long, long)
__asr(long long, long_long)

int
__undefined_shift_size(int x, int s);

#define asr(__x,__s) ((sizeof(__x) == sizeof(char)) ?                   \
                      (__typeof(__x))__asr_signed_char(__x, __s) :      \
                      (sizeof(__x) == sizeof(short)) ?                  \
                      (__typeof(__x))__asr_short(__x, __s) :            \
                      (sizeof(__x) == sizeof(int)) ?                    \
                      (__typeof(__x))__asr_int(__x, __s) :              \
                      (sizeof(__x) == sizeof(long)) ?                   \
                      (__typeof(__x))__asr_long(__x, __s) :             \
                      (sizeof(__x) == sizeof(long long)) ?              \
                      (__typeof(__x))__asr_long_long(__x, __s):         \
                      __undefined_shift_size(__x, __s))

int
main(void)
{
	int x = -35;
	int y;
	char	buf[256];
	int	errors = 0;

#if !defined(__PICOLIBC__) || defined(__MB_CAPABLE)
        if (!setlocale(LC_CTYPE, "C.UTF-8")) {
            printf("setlocale(LC_CTYPE, \"C.UTF-8\") failed\n");
            return 1;
        }
#endif
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

#ifndef NO_WIDE_IO
        unsigned wt;
        for (wt = 0; wtest[wt].str; wt++) {
            void *extra;
            int wtr = swscanf(wtest[wt].str, wtest[wt].fmt, &extra);
            if (wtr != wtest[wt].expect) {
                printf("%d str %ls fmt %ls expected %d got %d\n", wt,
                       wtest[wt].str, wtest[wt].fmt, wtest[wt].expect, wtr);
                ++errors;
            }
        }
#ifndef NO_MULTI_BYTE
        {
            wchar_t c;
            char test_val[] = "㌰";
            int i = sscanf(test_val, "%lc", &c);
            if (i != 1 || c != L'㌰') {
                printf("%d: %lc != %s or %d != 1\n", __LINE__, (wint_t) c, test_val, i);
                ++errors;
            }
            wchar_t wtest_val[] = L"㌰";
            char c_mb[MB_LEN_MAX+1] = {0};
            i = swscanf(wtest_val, L"%c", c_mb);
            if (i != 1 || strcmp(c_mb, test_val) != 0) {
                printf("%d: %s != %s or %d != 1\n", __LINE__, c_mb, test_val, i);
                ++errors;
            }
        }
#endif
#endif

#if !defined(__IO_NO_FLOATING_POINT)
        printf("checking basic floating point\n");
	sprintf(buf, "%g", printf_float(0.0f));
	if (strcmp(buf, "0") != 0) {
		printf("0: wanted \"0\" got \"%s\"\n", buf);
		errors++;
		fflush(stdout);
	}
#endif

#ifndef NO_POS_ARGS
        printf("checking pos args\n");
        x = y = 0;
        int r = sscanf("3 4", "%2$d %1$d", &x, &y);
        if (x != 4 || y != 3 || r != 2) {
            printf("pos: wanted %d %d (ret %d) got %d %d (ret %d)\n", 4, 3, 2, x, y, r);
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
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
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
        printf("checking binary format\n");
#define VERIFY_BINARY(prefix) VERIFY(prefix, "b")
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif
#else
#define VERIFY_BINARY(prefix)
#endif

#define CHECK_RT(type, prefix) do {                                     \
        for (x = 0; x < (int) (sizeof(type) * 8); x++) {                \
                type v = (type) (((uint64_t) 0x123456789abcdef0LL >> (64 - sizeof(type) * 8)) >> x); \
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

#define CHECK_RTI(type, prefix) do {                                     \
        for (x = 0; x < (int) (sizeof(type) * 8); x++) {                \
                type v = asr((type) ((uint64_t) 0x123456789abcdef0LL >> (64 - sizeof(type) * 8)), x); \
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

#ifndef __NANO_FORMATTED_IO
	CHECK_RT(unsigned char, "hh");
#endif
	CHECK_RT(unsigned short, "h");
        CHECK_RT(unsigned int, "");
        CHECK_RT(unsigned long, "l");
#ifndef NO_LONG_LONG
        printf("checking long long\n");
        CHECK_RT(unsigned long long, "ll");
#endif
#ifndef NO_C99_FORMATS
        printf("checking c99 formats\n");
#ifdef NO_LONG_LONG
	if (sizeof(intmax_t) <= sizeof(long))
#endif
	{
	        CHECK_RTI(intmax_t, "j");
	}
        CHECK_RT(size_t, "z");
        CHECK_RTI(ptrdiff_t, "t");
#endif

        {
            static int i_addr = 12;
            void *v = &i_addr;
            void *r = (void *) -1;
            VERIFY("", "p");
        }
#if !defined(__IO_NO_FLOATING_POINT)

        printf("checking floating point\n");
#ifndef NO_FLOAT_EXACT
        for (x = 0; x < 37; x++) {
                float_type m, n, a, b, c;
                float_type pow_val = int_exp10(x);

                m = 1 / pow_val;
                n = m / (float_type) 2.0;

                a = n;
                b = n;
                c = m;

                /* Make sure the values are on the right side of the rounding value */
                for (y = 0; y < 5; y++) {
                        a = nextafter(a, 2.0);
                        b = nextafter(b, 0.0);
                        c = nextafter(c, 0.0);
                }

                /*
                 * A value greater than 5 just past the last digit
                 * rounds up to 0.0...1
                 */
                sprintf(buf, "%.*f", x, printf_float(a));
                if (x == 0)
                        errors += check_float(">= 0.5", a, buf, "1", x, "");
                else
                        errors += check_float(">= 0.5", a, buf, "0.", x-1, "1");

                /*
                 * A value greater than 5 in the last digit
                 * rounds to 0.0...5
                 */
                sprintf(buf, "%.*f", x+1, printf_float(a));
                errors += check_float(">= 0.5", a, buf, "0.", x, "5");

                /*
                 * A value less than 5 just past the last digit
                 * rounds down to 0.0...0
                 */
                sprintf(buf, "%.*f", x, printf_float(b));
                if (x == 0)
                        errors += check_float("< 0.5", b, buf, "0", x, "");
                else
                        errors += check_float("< 0.5", b, buf, "0.", x, "");

                /*
                 * A value less than 5 in the last digit
                 * rounds to 0.0...5
                 */
                sprintf(buf, "%.*f", x+1, printf_float(b));
                errors += check_float("< 0.5", b, buf, "0.", x, "5");

                /*
                 * A value less than 1 in the last digit
                 * rounds to 0.0...1
                 */
                sprintf(buf, "%.*f", x, printf_float(c));
                if (x == 0)
                        errors += check_float("< 1", c, buf, "1", x, "");
                else
                        errors += check_float("< 1", c, buf, "0.", x-1, "1");
        }
#endif

	for (x = -37; x <= 37; x++)
	{
                float_type r;
		unsigned t;
		for (t = 0; t < sizeof(test_vals)/sizeof(test_vals[0]); t++) {

			float_type v = (float_type) test_vals[t] * int_exp10(x);
			float_type e;

			sprintf(buf, "%.55f", printf_float(v));
			sscanf(buf, scanf_format, &r);
			e = fabs(v-r) / v;
			if (e > (float_type) ERROR_MAX) {
				printf("\tf %3d: wanted %.7e %a got %.7e %a (error %.7e %a, buf %s)\n", x,
				       printf_float(v), printf_float(v),
                                       printf_float(r), printf_float(r),
                                       printf_float(e), printf_float(e),
                                       buf);
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

#ifndef NO_C99_FORMATS
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
#ifndef NO_C99_FORMATS
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
        printf("errors %d\n", errors);
	fflush(stdout);
	return errors;
}
