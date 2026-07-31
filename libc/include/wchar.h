/*
Copyright (c) 2001 Alexey Zelkin <phantom@FreeBSD.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
#ifndef _WCHAR_H_
#define _WCHAR_H_

#include <sys/cdefs.h>
#define __need_size_t
#define __need_wchar_t
#define __need_wint_t
#define __need_NULL
#include <stddef.h>

/* For __mbstate_t definition. */
#include <sys/_types.h>
#define __need___va_list
#include <stdarg.h>

#ifndef WEOF
#define WEOF ((wint_t) - 1)
#endif

/* This must match definition in <stdint.h> */
#ifndef WCHAR_MIN
#ifdef __WCHAR_MIN__
#define WCHAR_MIN __WCHAR_MIN__
#elif defined(__WCHAR_UNSIGNED__) || (L'\0' - 1 > 0)
#define WCHAR_MIN (0 + L'\0')
#else
#define WCHAR_MIN (-0x7fffffff - 1 + L'\0')
#endif
#endif

/* This must match definition in <stdint.h> */
#ifndef WCHAR_MAX
#ifdef __WCHAR_MAX__
#define WCHAR_MAX __WCHAR_MAX__
#elif defined(__WCHAR_UNSIGNED__) || (L'\0' - 1 > 0)
#define WCHAR_MAX (0xffffffffu + L'\0')
#else
#define WCHAR_MAX (0x7fffffff + L'\0')
#endif
#endif

#if __POSIX_VISIBLE >= 200809
#include <sys/_locale.h>
#endif

_BEGIN_STD_C

#ifndef ___FILE_DECLARED
typedef struct __file __FILE;
#define ___FILE_DECLARED
#endif

#if __POSIX_VISIBLE >= 200809 || _XSI_VISIBLE

#ifndef _FILE_DECLARED
typedef __FILE FILE;
#define _FILE_DECLARED
#endif

#ifndef _WCTYPE_DECLARED
typedef __wctype_t wctype_t;
#define _WCTYPE_DECLARED
#endif

#endif
/* As required by C, declare tm as incomplete type.
   The actual definition is in time.h. */
struct tm;

#ifndef _MBSTATE_DECLARED
typedef __mbstate_t mbstate_t;
#define _MBSTATE_DECLARED
#endif

wint_t btowc(int) __picolibc_export;
wint_t fgetwc(__FILE *) __picolibc_export;
#if __GNU_VISIBLE
wint_t fgetwc_unlocked(__FILE *) __picolibc_export;
wint_t ungetwc_unlocked(wint_t, __FILE *) __picolibc_export;
#endif
wchar_t *fgetws(wchar_t * __restrict, int, __FILE * __restrict) __picolibc_export;
#if __GNU_VISIBLE
wchar_t *fgetws_unlocked(wchar_t * __restrict, int, __FILE * __restrict) __picolibc_export;
#endif
wint_t fputwc(wchar_t, __FILE *) __picolibc_export;
#if __GNU_VISIBLE
wint_t fputwc_unlocked(wchar_t, __FILE *) __picolibc_export;
#endif
int fputws(const wchar_t * __restrict, __FILE * __restrict) __picolibc_export;
#if __GNU_VISIBLE
int fputws_unlocked(const wchar_t * __restrict, __FILE * __restrict) __picolibc_export;
#endif
#if __ISO_C_VISIBLE >= 1999 || __XSI_VISIBLE >= 500
int fwide(__FILE *, int) __picolibc_export;
#endif
#if __ISO_C_VISIBLE >= 1999 || __XSI_VISIBLE >= 500
int fwprintf(__FILE * __restrict, const wchar_t * __restrict, ...) __picolibc_export;
int fwscanf(__FILE * __restrict, const wchar_t * __restrict, ...) __picolibc_export;
#endif
wint_t getwc(__FILE *) __picolibc_export;
#if __GNU_VISIBLE
wint_t getwc_unlocked(__FILE *) __picolibc_export;
#endif
wint_t getwchar(void) __picolibc_export;
#if __GNU_VISIBLE
wint_t getwchar_unlocked(void) __picolibc_export;
#endif
size_t mbrlen(const char * __restrict, size_t, mbstate_t * __restrict) __picolibc_export;
size_t mbrtowc(wchar_t * __restrict, const char * __restrict, size_t,
               mbstate_t * __restrict) __picolibc_export;
int    mbsinit(const mbstate_t *) __picolibc_export;
#if __POSIX_VISIBLE >= 200809
size_t mbsnrtowcs(wchar_t * __restrict, const char ** __restrict, size_t, size_t,
                  mbstate_t * __restrict) __picolibc_export;
#endif
size_t mbsrtowcs(wchar_t * __restrict, const char ** __restrict, size_t,
                 mbstate_t * __restrict) __picolibc_export;
#if __POSIX_VISIBLE >= 200809
__FILE *open_wmemstream(wchar_t **, size_t *) __picolibc_export;
#endif
wint_t putwc(wchar_t, __FILE *) __picolibc_export;
#if __GNU_VISIBLE
wint_t putwc_unlocked(wchar_t, __FILE *) __picolibc_export;
#endif
wint_t putwchar(wchar_t) __picolibc_export;
#if __GNU_VISIBLE
wint_t putwchar_unlocked(wchar_t) __picolibc_export;
#endif
#if __ISO_C_VISIBLE >= 1999 || __XSI_VISIBLE >= 500
int swprintf(wchar_t * __restrict, size_t, const wchar_t * __restrict, ...) __picolibc_export;
int swscanf(const wchar_t * __restrict, const wchar_t * __restrict, ...) __picolibc_export;
#endif
#if __GNU_VISIBLE
int __d_swprintf(wchar_t * __restrict, size_t, const wchar_t * __restrict, ...) __picolibc_export;
#endif
wint_t ungetwc(wint_t wc, __FILE *) __picolibc_export;
#if __ISO_C_VISIBLE >= 1999 || __XSI_VISIBLE >= 500
int vfwprintf(__FILE * __restrict, const wchar_t * __restrict, __gnuc_va_list) __picolibc_export;
int vfwscanf(__FILE * __restrict, const wchar_t * __restrict, __gnuc_va_list) __picolibc_export;
int vswprintf(wchar_t * __restrict, size_t, const wchar_t * __restrict,
              __gnuc_va_list) __picolibc_export;
int vswscanf(const wchar_t * __restrict, const wchar_t * __restrict,
             __gnuc_va_list) __picolibc_export;
int vwprintf(const wchar_t * __restrict, __gnuc_va_list) __picolibc_export;
int vwscanf(const wchar_t * __restrict, __gnuc_va_list) __picolibc_export;
#endif
#if __POSIX_VISIBLE >= 200809
wchar_t *wcpcpy(wchar_t * __restrict, const wchar_t * __restrict) __picolibc_export;
wchar_t *wcpncpy(wchar_t * __restrict, const wchar_t * __restrict, size_t) __picolibc_export;
#endif
size_t wcrtomb(char * __restrict, wchar_t, mbstate_t * __restrict) __picolibc_export;
#if __POSIX_VISIBLE >= 200809
int wcscasecmp(const wchar_t *, const wchar_t *) __picolibc_export;
int wcscasecmp_l(const wchar_t *, const wchar_t *, locale_t) __picolibc_export;
#endif
wchar_t *wcscat(wchar_t * __restrict, const wchar_t * __restrict) __picolibc_export;
wchar_t *wcschr(const wchar_t *, wchar_t) __picolibc_export;
int      wcscmp(const wchar_t *, const wchar_t *) __picolibc_export;
int      wcscoll(const wchar_t *, const wchar_t *) __picolibc_export;
#if __POSIX_VISIBLE >= 200809
int wcscoll_l(const wchar_t *, const wchar_t *, locale_t) __picolibc_export;
#endif
wchar_t *wcscpy(wchar_t * __restrict, const wchar_t * __restrict) __picolibc_export;
size_t   wcscspn(const wchar_t *, const wchar_t *) __picolibc_export;

#if __POSIX_VISIBLE >= 200809
void     free(void *) __nothrow __picolibc_export;
wchar_t *wcsdup(const wchar_t *) __malloc_like __warn_unused_result __picolibc_export;
#endif
size_t wcsftime(wchar_t * __restrict, size_t, const wchar_t * __restrict,
                const struct tm * __restrict) __picolibc_export;
#if __GNU_VISIBLE
size_t wcsftime_l(wchar_t * __restrict, size_t, const wchar_t * __restrict,
                  const struct tm * __restrict, locale_t) __picolibc_export;
#endif
#if __BSD_VISIBLE
size_t wcslcat(wchar_t *, const wchar_t *, size_t) __picolibc_export;
size_t wcslcpy(wchar_t *, const wchar_t *, size_t) __picolibc_export;
#endif
size_t wcslen(const wchar_t *) __picolibc_export;
#if __POSIX_VISIBLE >= 200809
int wcsncasecmp(const wchar_t *, const wchar_t *, size_t) __picolibc_export;
int wcsncasecmp_l(const wchar_t *, const wchar_t *, size_t, locale_t) __picolibc_export;
#endif
wchar_t *wcsncat(wchar_t * __restrict, const wchar_t * __restrict, size_t) __picolibc_export;
int      wcsncmp(const wchar_t *, const wchar_t *, size_t) __picolibc_export;
wchar_t *wcsncpy(wchar_t * __restrict, const wchar_t * __restrict, size_t) __picolibc_export;
#if __POSIX_VISIBLE >= 200809
size_t wcsnlen(const wchar_t *, size_t) __picolibc_export;
size_t wcsnrtombs(char * __restrict, const wchar_t ** __restrict, size_t, size_t,
                  mbstate_t * __restrict) __picolibc_export;
#endif
wchar_t *wcspbrk(const wchar_t *, const wchar_t *) __picolibc_export;
wchar_t *wcsrchr(const wchar_t *, wchar_t) __picolibc_export;
size_t   wcsrtombs(char   *__restrict, const wchar_t   **__restrict, size_t,
                   mbstate_t   *__restrict) __picolibc_export;
size_t   wcsspn(const wchar_t *, const wchar_t *) __picolibc_export;
wchar_t *wcsstr(const wchar_t * __restrict, const wchar_t * __restrict) __picolibc_export;
double   wcstod(const wchar_t   *__restrict, wchar_t   **__restrict) __picolibc_export;
#if __GNU_VISIBLE
double wcstod_l(const wchar_t *, wchar_t **, locale_t) __picolibc_export;
#endif
#if __ISO_C_VISIBLE >= 1999
float wcstof(const wchar_t * __restrict, wchar_t ** __restrict) __picolibc_export;
#endif
#if __GNU_VISIBLE
float wcstof_l(const wchar_t *, wchar_t **, locale_t) __picolibc_export;
#endif
wchar_t *wcstok(wchar_t * __restrict, const wchar_t * __restrict,
                wchar_t ** __restrict) __picolibc_export;
long     wcstol(const wchar_t     *__restrict, wchar_t     **__restrict, int) __picolibc_export;
#if __GNU_VISIBLE
long wcstol_l(const wchar_t * __restrict, wchar_t ** __restrict, int, locale_t) __picolibc_export;
#endif
#if __ISO_C_VISIBLE >= 1999 && defined(__HAVE_LONG_DOUBLE)
long double wcstold(const wchar_t *, wchar_t **) __picolibc_export;
#endif
#if __GNU_VISIBLE && defined(__HAVE_LONG_DOUBLE)
long double wcstold_l(const wchar_t *, wchar_t **, locale_t) __picolibc_export;
#endif
#if __ISO_C_VISIBLE >= 1999
long long wcstoll(const wchar_t * __restrict, wchar_t ** __restrict, int) __picolibc_export;
#endif
#if __GNU_VISIBLE
long long wcstoll_l(const wchar_t * __restrict, wchar_t ** __restrict, int,
                    locale_t) __picolibc_export;
#endif
unsigned long wcstoul(const wchar_t * __restrict, wchar_t ** __restrict, int) __picolibc_export;
#if __GNU_VISIBLE
unsigned long wcstoul_l(const wchar_t * __restrict, wchar_t ** __restrict, int,
                        locale_t) __picolibc_export;
#endif
#if __ISO_C_VISIBLE >= 1999
unsigned long long wcstoull(const wchar_t * __restrict, wchar_t ** __restrict,
                            int) __picolibc_export;
#endif
#if __GNU_VISIBLE
unsigned long long wcstoull_l(const wchar_t * __restrict, wchar_t ** __restrict, int,
                              locale_t) __picolibc_export;
#endif
#if __XSI_VISIBLE
int wcswidth(const wchar_t *, size_t) __picolibc_export;
#endif
size_t wcsxfrm(wchar_t * __restrict, const wchar_t * __restrict, size_t) __picolibc_export;
#if __POSIX_VISIBLE >= 200809
size_t wcsxfrm_l(wchar_t * __restrict, const wchar_t * __restrict, size_t,
                 locale_t) __picolibc_export;
#endif
int wctob(wint_t) __picolibc_export;
#if __XSI_VISIBLE
int wcwidth(const wchar_t) __picolibc_export;
#endif
wchar_t *wmemchr(const wchar_t *, wchar_t, size_t) __picolibc_export;
int      wmemcmp(const wchar_t *, const wchar_t *, size_t) __picolibc_export;
wchar_t *wmemcpy(wchar_t * __restrict, const wchar_t * __restrict, size_t) __picolibc_export;
wchar_t *wmemmove(wchar_t *, const wchar_t *, size_t) __picolibc_export;
#if __GNU_VISIBLE
wchar_t *wmempcpy(wchar_t * __restrict, const wchar_t * __restrict, size_t) __picolibc_export;
#endif
wchar_t *wmemset(wchar_t *, wchar_t, size_t) __picolibc_export;
#if __ISO_C_VISIBLE >= 1999 || __XSI_VISIBLE >= 500
int wprintf(const wchar_t * __restrict, ...) __picolibc_export;
int wscanf(const wchar_t * __restrict, ...) __picolibc_export;
#endif

#if __STDC_WANT_LIB_EXT1__ == 1
#ifndef __STDC_LIB_EXT1__
#define __STDC_LIB_EXT1__ 1
#endif

#include <sys/_types.h>

#ifndef _ERRNO_T_DEFINED
typedef __errno_t errno_t;
#define _ERRNO_T_DEFINED
#endif

#ifndef _RSIZE_T_DEFINED
typedef __rsize_t rsize_t;
#define _RSIZE_T_DEFINED
#endif
#endif

_END_STD_C

#if __SSP_FORTIFY_LEVEL > 0
#include <ssp/wchar.h>
#endif

#endif /* _WCHAR_H_ */
