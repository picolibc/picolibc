/*
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.
All or some portions of this file are derived from material licensed
to the University of California by American Telephone and Telegraph
Co. or Unix System Laboratories, Inc. and are reproduced herein with
the permission of UNIX System Laboratories, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
/*
 * string.h
 *
 * Definitions for memory and string functions.
 */

#ifndef _STRING_H_
#define _STRING_H_

#include <sys/cdefs.h>

#define __need_size_t
#define __need_NULL
#include <stddef.h>

#if __POSIX_VISIBLE >= 200809
#include <sys/_locale.h>
#endif

#if __BSD_VISIBLE
#include <strings.h>
#endif

_BEGIN_STD_C

/* There are two common basename variants.  If you do NOT #include <libgen.h>
   and you do

     #define _GNU_SOURCE
     #include <string.h>

   you get the GNU version.  Otherwise you get the POSIX versionfor which you
   should #include <libgen.h>i for the function prototype.  POSIX requires that
   #undef basename will still let you invoke the underlying function.  However,
   this also implies that the POSIX version is used in this case.  That's made
   sure here. */
#if __GNU_VISIBLE && !defined(basename)
#ifdef __ASMNAME
#define basename basename
char * __nonnull((1)) basename(const char *) _ASMNAME("__gnu_basename");
#else
#define basename(s) (__gnu_basename(s))
#endif
#endif

#if __MISC_VISIBLE || __POSIX_VISIBLE
void *memccpy(void * __restrict, const void * __restrict, int, size_t) __picolibc_export;
#endif
void *memchr(const void *, int, size_t) __picolibc_export;
int   memcmp(const void *, const void *, size_t) __picolibc_export;
void *memcpy(void * __restrict, const void * __restrict, size_t) __picolibc_export;
#if __GNU_VISIBLE
void *memmem(const void *, size_t, const void *, size_t) __picolibc_export;
#endif
void *memmove(void *, const void *, size_t) __picolibc_export;
#if __GNU_VISIBLE
void *mempcpy(void *, const void *, size_t) __picolibc_export;
void *memrchr(const void *, int, size_t) __picolibc_export;
#endif
void *memset(void *, int, size_t) __picolibc_export;
#if __ISO_C_VISIBLE >= 2023
void *memset_explicit(void *, int, size_t) __picolibc_export;
#endif
#if __GNU_VISIBLE
void *rawmemchr(const void *, int) __picolibc_export;
#endif
#if __POSIX_VISIBLE >= 200809
char *stpcpy(char * __restrict, const char * __restrict) __picolibc_export;
char *stpncpy(char * __restrict, const char * __restrict, size_t) __picolibc_export;
#endif
#if __GNU_VISIBLE
char *strcasestr(const char *, const char *) __picolibc_export;
#endif
char *strcat(char * __restrict, const char * __restrict) __picolibc_export;
char *strchr(const char *, int) __picolibc_export;
#if __GNU_VISIBLE
char *strchrnul(const char *, int) __picolibc_export;
#endif
int strcmp(const char *, const char *) __picolibc_export;
int strcoll(const char *, const char *) __picolibc_export;
#if __POSIX_VISIBLE >= 200809
int strcoll_l(const char *, const char *, locale_t) __picolibc_export;
#endif
char  *strcpy(char  *__restrict, const char  *__restrict) __picolibc_export;
size_t strcspn(const char *, const char *) __picolibc_export;
#if __MISC_VISIBLE || __POSIX_VISIBLE >= 200809 || __XSI_VISIBLE >= 4
void  free(void *) __nothrow __picolibc_export; /* for __malloc_like */
char *strdup(const char *) __malloc_like __warn_unused_result __picolibc_export;
#endif
#if __GNU_VISIBLE && defined(__GNUC__)
#define strdupa(__s)                                          \
    (__extension__({                                          \
        const char *__sin = (__s);                            \
        size_t      __len = strlen(__sin) + 1;                \
        char       *__sout = (char *)__builtin_alloca(__len); \
        (char *)memcpy(__sout, __sin, __len);                 \
    }))
#define strndupa(__s, __n)                                    \
    (__extension__({                                          \
        const char *__sin = (__s);                            \
        size_t      __len = strnlen(__sin, (__n)) + 1;        \
        char       *__sout = (char *)__builtin_alloca(__len); \
        __sout[__len - 1] = '\0';                             \
        (char *)memcpy(__sout, __sin, __len - 1);             \
    }))
#endif /* __GNU_VISIBLE && __GNUC__ */
char *strerror(int) __picolibc_export;
#if __POSIX_VISIBLE >= 200809
char *strerror_l(int, locale_t) __picolibc_export;
#endif
/* There are two common strerror_r variants.  If you request
   _GNU_SOURCE, you get the GNU version; otherwise you get the POSIX
   version.  POSIX requires that #undef strerror_r will still let you
   invoke the underlying function, but that requires gcc support.  */
#if __GNU_VISIBLE
char *strerror_r(int, char *, size_t) __picolibc_export;
#elif __POSIX_VISIBLE >= 200112
#ifdef __GNUC__
int strerror_r(int, char *, size_t) _ASMNAME("__xpg_strerror_r") __picolibc_export;
#else
int __xpg_strerror_r(int, char *, size_t) __picolibc_export;
#define strerror_r __xpg_strerror_r
#endif
#endif
#if __GNU_VISIBLE
const char *strerrorname_np(int errnum) __picolibc_export;
const char *strerrordesc_np(int errnum) __picolibc_export;
#endif
#if __BSD_VISIBLE
size_t strlcat(char *, const char *, size_t) __picolibc_export;
#endif
size_t strlen(const char *) __picolibc_export;
#if __BSD_VISIBLE
size_t strlcpy(char *, const char *, size_t) __picolibc_export;
#endif
#if __MISC_VISIBLE
char *strlwr(char *) __picolibc_export;
#endif
char *strncat(char * __restrict, const char * __restrict, size_t) __picolibc_export;
int   strncmp(const char *, const char *, size_t) __picolibc_export;
char *strncpy(char * __restrict, const char * __restrict, size_t) __picolibc_export;
#if __POSIX_VISIBLE >= 200809
char                                    *
strndup(const char *, size_t)
__malloc_like __warn_unused_result __picolibc_export;
#endif
#if __POSIX_VISIBLE >= 200809 || __ZEPHYR_VISIBLE
size_t strnlen(const char *, size_t) __picolibc_export;
#endif
#if __BSD_VISIBLE
char        *
strnstr(const char *, const char *, size_t)
__pure __picolibc_export;
#endif
char *strpbrk(const char *, const char *) __picolibc_export;
char *strrchr(const char *, int) __picolibc_export;
#if __BSD_VISIBLE
char *strsep(char **, const char *) __picolibc_export;
#endif
#if __POSIX_VISIBLE >= 200809
char *strsignal(int __signo) __picolibc_export;
#endif
size_t strspn(const char *, const char *) __picolibc_export;
char  *strstr(const char *, const char *) __picolibc_export;
char  *strtok(char  *__restrict, const char  *__restrict) __picolibc_export;
#if __MISC_VISIBLE || __POSIX_VISIBLE || __ZEPHYR_VISIBLE
char *strtok_r(char * __restrict, const char * __restrict, char ** __restrict) __picolibc_export;
#endif
#if __MISC_VISIBLE
char *strupr(char *) __picolibc_export;
#endif
#if __GNU_VISIBLE
int strverscmp(const char *, const char *) __picolibc_export;
#endif
size_t strxfrm(char * __restrict, const char * __restrict, size_t) __picolibc_export;
#if __POSIX_VISIBLE >= 200809
size_t strxfrm_l(char * __restrict, const char * __restrict, size_t, locale_t) __picolibc_export;
#endif
#if __BSD_VISIBLE
int timingsafe_bcmp(const void *, const void *, size_t) __picolibc_export;
int timingsafe_memcmp(const void *, const void *, size_t) __picolibc_export;
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

errno_t memcpy_s(void * __restrict, rsize_t, const void * __restrict, rsize_t) __picolibc_export;
errno_t memset_s(void *, rsize_t, int, rsize_t) __picolibc_export;
errno_t memmove_s(void *, rsize_t, const void *, rsize_t) __picolibc_export;
errno_t strcpy_s(char * __restrict, rsize_t, const char * __restrict) __picolibc_export;
errno_t strcat_s(char * __restrict, rsize_t, const char * __restrict) __picolibc_export;
errno_t strncpy_s(char * __restrict, rsize_t, const char * __restrict, rsize_t) __picolibc_export;
errno_t strncat_s(char * __restrict, rsize_t, const char * __restrict, rsize_t) __picolibc_export;
size_t  strnlen_s(const char *, size_t) __picolibc_export;
errno_t strerror_s(char *, rsize_t, errno_t) __picolibc_export;
size_t  strerrorlen_s(errno_t) __picolibc_export;
#endif

#include <sys/string.h>

_END_STD_C

#if __SSP_FORTIFY_LEVEL > 0
#include <ssp/string.h>
#endif

#endif /* _STRING_H_ */
