/* Copyright © 2013 Bart Massey */
/* This program is licensed under the GPL version 2 or later.
   Please see the file COPYING.GPL2 in this distribution for
   license terms. */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>

#ifndef TINY_STDIO
#define printf_float(x) ((double) (x))
#ifdef _NANO_FORMATTED_IO
#ifndef NO_FLOATING_POINT
extern int _printf_float();
extern int _scanf_float();

int (*_reference_printf_float)() = _printf_float;
int (*_reference_scanf_float)() = _scanf_float;
#define TEST_ASPRINTF
#endif
#endif
#endif

#define PRINTF_BUF_SIZE 512

static char buf[PRINTF_BUF_SIZE];
static wchar_t wbuf[PRINTF_BUF_SIZE];

static void failmsg(int serial, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    printf("test %d failed: ", serial);
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
/* 'bsize' is used directly with malloc/realloc which confuses -fanalyzer */
#pragma GCC diagnostic ignored "-Wanalyzer-va-arg-type-mismatch"
#pragma GCC diagnostic ignored "-Wanalyzer-va-list-exhausted"
#endif

static int test(int serial, char *expect, char *fmt, ...) {
    va_list ap;
    char *abuf = NULL;
    va_start(ap, fmt);
    int n;
#ifdef TEST_ASPRINTF
    int an;
    va_list aap;
    va_copy(aap, ap);
#endif
#ifndef NO_FLOATING_POINT
# ifdef _HAS_IO_FLOAT
    uint32_t dv;
# else
    double dv;
# endif
#ifndef NO_LONG_DOUBLE
    long double ldv;
#endif
    char *star;
    char *long_double;
#endif
    switch (fmt[strlen(fmt)-1]) {
    case 'e':
    case 'E':
    case 'f':
    case 'F':
    case 'g':
    case 'G':
    case 'a':
    case 'A':
#ifdef NO_FLOATING_POINT
	    return 0;
#else
	    star = strchr(fmt, '*');
            long_double = strchr(fmt, 'L');
	    if (star) {
		    if (strchr(star+1, '*')) {
			    int iv1 = va_arg(ap, int);
			    int iv2 = va_arg(ap, int);
#ifndef NO_LONG_DOUBLE
                            if (long_double) {
                                    ldv = va_arg(ap, long double);
                                    n = snprintf(buf, PRINTF_BUF_SIZE, fmt, iv1, iv2, ldv);
#ifdef TEST_ASPRINTF
                                    an = asprintf(&abuf, fmt, iv1, iv2, ldv);
#endif
                            } else
#endif
                            {
                                    dv = va_arg(ap, __typeof(dv));
                                    n = snprintf(buf, PRINTF_BUF_SIZE, fmt, iv1, iv2, dv);
#ifdef TEST_ASPRINTF
                                    an = asprintf(&abuf, fmt, iv1, iv2, dv);
#endif
                            }
		    } else  {
			    int iv = va_arg(ap, int);
#ifndef NO_LONG_DOUBLE
                            if (long_double) {
                                    ldv = va_arg(ap, long double);
                                    n = snprintf(buf, PRINTF_BUF_SIZE, fmt, iv, ldv);
#ifdef TEST_ASPRINTF
                                    an = asprintf(&abuf, fmt, iv, ldv);
#endif
                            } else
#endif
                            {
                                    dv = va_arg(ap, __typeof(dv));
                                    n = snprintf(buf, PRINTF_BUF_SIZE, fmt, iv, dv);
#ifdef TEST_ASPRINTF
                                    an = asprintf(&abuf, fmt, iv, dv);
#endif
                            }
		    }
	    } else {
#ifndef NO_LONG_DOUBLE
                    if (long_double) {
                            ldv = va_arg(ap, long double);
                            n = snprintf(buf, PRINTF_BUF_SIZE, fmt, ldv);
#ifdef TEST_ASPRINTF
                            an = asprintf(&abuf, fmt, ldv);
#endif
                    } else
#endif
                    {
                            dv = va_arg(ap, __typeof(dv));
                            n = snprintf(buf, PRINTF_BUF_SIZE, fmt, dv);
#ifdef TEST_ASPRINTF
                            an = asprintf(&abuf, fmt, dv);
#endif
                    }
	    }
	    break;
#endif
    default:
	    n = vsnprintf(buf, PRINTF_BUF_SIZE, fmt, ap);
#ifdef TEST_ASPRINTF
	    an = vasprintf(&abuf, fmt, aap);
#endif
	    break;
    }
    va_end(ap);
#ifdef TEST_ASPRINTF
    va_end(aap);
#endif
//    printf("serial %d expect \"%s\" got \"%s\"\n", serial, expect, buf);
    if (n >= PRINTF_BUF_SIZE) {
        failmsg(serial, "buffer overflow");
	free(abuf);
        return 1;
    }
    if (n != (int) strlen(expect)) {
        failmsg(serial, "expected \"%s\" (%d), got \"%s\" (%d)",
		expect, (int) strlen(expect), buf, n);
	free(abuf);
        return 1;
    }
    if (strcmp(buf, expect)) {
        failmsg(serial, "expected \"%s\", got \"%s\"", expect, buf);
	free(abuf);
        return 1;
    }
#ifdef TEST_ASPRINTF
    if (an != n) {
	failmsg(serial, "asprintf return %d sprintf return %d\n", an, n);
	free(abuf);
	return 1;
    }
    if (strcmp(abuf, buf)) {
	failmsg(serial, "sprintf return %s asprintf return %s\n", buf, abuf);
	free(abuf);
	return 1;
    }
    free(abuf);
#endif
    return 0;
}

static void failmsgw(int serial, wchar_t *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    printf("test %d failed: ", serial);
    static wchar_t f_wbuf[PRINTF_BUF_SIZE];
    static char f_buf[PRINTF_BUF_SIZE];
    vswprintf(f_wbuf, PRINTF_BUF_SIZE, fmt, ap);
    wcstombs(f_buf, f_wbuf, PRINTF_BUF_SIZE);
    printf("%s\n", f_buf);
    va_end(ap);
}

static int testw(int serial, wchar_t *expect, wchar_t *fmt, ...) {
    va_list ap;
    wchar_t *abuf = NULL;
    va_start(ap, fmt);
    int n;
#ifdef TEST_ASPRINTF
    int an;
    va_list aap;
    va_copy(aap, ap);
#endif
#ifndef NO_FLOATING_POINT
# ifdef _HAS_IO_FLOAT
    uint32_t dv;
# else
    double dv;
# endif
    wchar_t *star;
#endif
    switch (fmt[wcslen(fmt)-1]) {
    case 'e':
    case 'E':
    case 'f':
    case 'F':
    case 'g':
    case 'G':
    case 'a':
    case 'A':
#ifdef NO_FLOATING_POINT
	    return 0;
#else
	    star = wcschr(fmt, '*');
	    if (star) {
		    if (wcschr(star+1, '*')) {
			    int iv1 = va_arg(ap, int);
			    int iv2 = va_arg(ap, int);
			    dv = va_arg(ap, __typeof(dv));
			    n = swprintf(wbuf, PRINTF_BUF_SIZE, fmt, iv1, iv2, dv);
		    } else  {
			    int iv = va_arg(ap, int);
			    dv = va_arg(ap, __typeof(dv));
			    n = swprintf(wbuf, PRINTF_BUF_SIZE, fmt, iv, dv);
		    }
	    } else {
		    dv = va_arg(ap, __typeof(dv));
		    n = swprintf(wbuf, PRINTF_BUF_SIZE, fmt, dv);
	    }
	    break;
#endif
    default:
	    n = vswprintf(wbuf, PRINTF_BUF_SIZE, fmt, ap);
	    break;
    }
    va_end(ap);
//    printf("serial %d expect \"%s\" got \"%s\"\n", serial, expect, wbuf);
    if (n >= PRINTF_BUF_SIZE) {
        failmsgw(serial, L"buffer overflow");
	free(abuf);
        return 1;
    }
    if (n != (int) wcslen(expect)) {
        failmsgw(serial, L"expected \"%s\" (%d), got \"%s\" (%d)",
		expect, wcslen(expect), wbuf, n);
	free(abuf);
        return 1;
    }
    if (wcscmp(wbuf, expect)) {
        failmsgw(serial, L"expected \"%ls\", got \"%ls\"", expect, wbuf);
	free(abuf);
        return 1;
    }
    return 0;
}

int main(void) {
    int result = 0;
#if !defined(__PICOLIBC__) || defined(_MB_CAPABLE)
    if (!setlocale(LC_CTYPE, "C.UTF-8")) {
        printf("setlocale(LC_CTYPE, \"C.UTF-8\") failed\n");
        return 1;
    }
#endif
#include "testcases.c"
    return result;
}
