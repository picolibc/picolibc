/* Copyright (c) 2002,2005, Joerg Wunsch
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/* $Id: stdio_private.h 847 2005-09-06 18:49:15Z joerg_wunsch $ */

#ifndef _STDIO_PRIVATE_H_
#define _STDIO_PRIVATE_H_

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <wchar.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <stdio-bufio.h>
#include <sys/lock.h>

struct __file_str {
	struct __file file;	/* main file struct */
        char	*pos;		/* current buffer position */
        char    *end;           /* end of buffer */
        size_t  size;           /* size of allocated storage */
        bool    alloc;          /* current storage was allocated */
};

int
__file_str_get(FILE *stream);

int
__file_wstr_get(FILE *stream);

int
__file_str_put(char c, FILE *stream);

int
__file_str_put_alloc(char c, FILE *stream);

extern const char __match_inf[];
extern const char __match_inity[];
extern const char __match_nan[];

/* Returns 'true' if prefix of input matches pattern, independent of
 * case. pattern is only upper case letters.
 */
bool __matchcaseprefix(const char *input, const char *pattern);

/*
 * It is OK to discard the "const" qualifier here.  f.buf is
 * non-const as in the generic case, this buffer is obtained
 * by malloc().  In the scanf case however, the buffer is
 * really only be read (by getc()), and as this our FILE f we
 * be discarded upon exiting sscanf(), nobody will ever get
 * a chance to get write access to it again.
 */
#define FDEV_SETUP_STRING_READ(_s) {		\
		.file = {			\
			.flags = __SRD,		\
                        .get = __file_str_get,  \
                        __LOCK_INIT_NONE        \
		},				\
		.pos = (char *) (_s)		\
	}

#define FDEV_SETUP_WSTRING_READ(_s) {		\
		.file = {			\
			.flags = __SRD,		\
			.get = __file_wstr_get,	\
                        __LOCK_INIT_NONE        \
		},				\
                .pos = (char *) (_s),           \
                .end = (char *) (_s)            \
	}

#define FDEV_STRING_WRITE_END(_s, _n) \
    (((int) (_n) < 0) ? NULL : ((_n) ? (_s) + (_n)-1 : (_s)))

#define FDEV_SETUP_STRING_WRITE(_s, _end) {	\
		.file = {			\
			.flags = __SWR,		\
			.put = __file_str_put,	\
                        __LOCK_INIT_NONE        \
		},				\
		.pos = (_s),			\
                .end = (_end),                  \
	}

#define FDEV_SETUP_STRING_ALLOC() {  \
		.file = {			\
			.flags = __SWR,		\
			.put = __file_str_put_alloc,	\
                        __LOCK_INIT_NONE        \
		},				\
		.pos = NULL,			\
                .end = NULL,                    \
                .size = 0,                      \
                .alloc = false,                 \
	}

#define FDEV_SETUP_STRING_ALLOC_BUF(_buf, _size) {  \
		.file = {			\
			.flags = __SWR,		\
			.put = __file_str_put_alloc,	\
                        __LOCK_INIT_NONE        \
		},				\
		.pos = _buf,			\
                .end = (char *) (_buf) + (_size), \
                .size = _size,                  \
                .alloc = false,                 \
	}

#define _FDEV_BUFIO_FD(bf) ((int)((intptr_t) (bf)->ptr))

#define IO_VARIANT_IS_FLOAT(v)        ((v) == __IO_VARIANT_FLOAT || (v) == __IO_VARIANT_DOUBLE)

/*
 * While there are notionally two different ways to invoke the
 * callbacks (one with an int and the other with a pointer), they are
 * functionally identical on many architectures. Check for that and
 * skip the extra code.
 */
#if __SIZEOF_POINTER__ == __SIZEOF_INT__ || defined(__x86_64) || defined(__arm__) || defined(__riscv)
#define BUFIO_ABI_MATCHES
#endif

#if defined(__has_feature)
#if __has_feature(undefined_behavior_sanitizer)
#undef BUFIO_ABI_MATCHES
#endif
#endif

/* Buffered I/O routines for tiny stdio */

static inline ssize_t bufio_read(struct __file_bufio *bf, void *buf, size_t count)
{
#ifndef BUFIO_ABI_MATCHES
    if (!(bf->bflags & __BFPTR))
        return (bf->read_int)(_FDEV_BUFIO_FD(bf), buf, count);
#endif
    return (bf->read_ptr)((void *) bf->ptr, buf, count);
}

static inline ssize_t bufio_write(struct __file_bufio *bf, const void *buf, size_t count)
{
#ifndef BUFIO_ABI_MATCHES
    if (!(bf->bflags & __BFPTR))
        return (bf->write_int)(_FDEV_BUFIO_FD(bf), buf, count);
#endif
    return (bf->write_ptr)((void *) bf->ptr, buf, count);
}

static inline __off_t bufio_lseek(struct __file_bufio *bf, __off_t offset, int whence)
{
#ifndef BUFIO_ABI_MATCHES
    if (!(bf->bflags & __BFPTR)) {
        if (bf->lseek_int)
            return (bf->lseek_int)(_FDEV_BUFIO_FD(bf), offset, whence);
    } else
#endif
    {
        if (bf->lseek_ptr)
            return (bf->lseek_ptr)((void *) bf->ptr, offset, whence);
    }
    return _FDEV_ERR;
}

static inline int bufio_close(struct __file_bufio *bf)
{
    int ret = 0;
#ifndef BUFIO_ABI_MATCHES
    if (!(bf->bflags & __BFPTR)) {
        if (bf->close_int)
            ret = (bf->close_int)(_FDEV_BUFIO_FD(bf));
    } else
#endif
    {
        if (bf->close_ptr)
            ret = (bf->close_ptr)((void *) bf->ptr);
    }
    return ret;
}

#define FDEV_SETUP_POSIX(fd, buf, size, rwflags, bflags)      \
        FDEV_SETUP_BUFIO(fd, buf, size,                       \
                         read, write,                         \
                         lseek, close, rwflags, bflags)

int
__stdio_flags (const char *mode, int *optr);

#ifdef __STDIO_LOCKING
void __flockfile_init(FILE *f);
#define __LOCK_NONE     ((_LOCK_RECURSIVE_T) (uintptr_t) 1)
#define __LOCK_INIT_NONE        .lock = __LOCK_NONE
#else
#define __LOCK_INIT_NONE
#endif

#define __funlock_return(f, v) do { __funlockfile(f); return (v); } while(0)

static inline void __flockfile(FILE *f) {
	(void) f;
#ifdef __STDIO_LOCKING
	if (!f->lock)
            __flockfile_init(f);
        if (f->lock != __LOCK_NONE)
            __lock_acquire_recursive(f->lock);
#endif
}

static inline void __funlockfile(FILE *f) {
	(void) f;
#ifdef __STDIO_LOCKING
        if (f->lock != __LOCK_NONE)
            __lock_release_recursive(f->lock);
#endif
}

static inline void __flockfile_close(FILE *f) {
	(void) f;
#ifdef __STDIO_LOCKING
        if (f->lock && f->lock != __LOCK_NONE)
            __lock_close(f->lock);
#endif
}

/* Silence santizer errors when adding/subtracting 0 to a NULL pointer */
#ifdef __clang__
#define POINTER_MINUS(a,b)     ((__typeof(a)) ((uintptr_t) (a) - (b) * sizeof((*a))))
#define POINTER_PLUS(a,b)      ((__typeof(a)) ((uintptr_t) (a) + (b) * sizeof((*a))))
#else
#define POINTER_MINUS(a,b)     ((a) - (b))
#define POINTER_PLUS(a,b)      ((a) + (b))
#endif

int	__d_vfprintf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(printf, 2, 0);
int	__f_vfprintf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(printf, 2, 0);
int	__i_vfprintf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(printf, 2, 0);
int	__l_vfprintf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(printf, 2, 0);
int	__m_vfprintf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(printf, 2, 0);

int	__d_sprintf(char *__s, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 2, 0);
int	__f_sprintf(char *__s, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 2, 0);
int	__i_sprintf(char *__s, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 2, 0);
int	__l_sprintf(char *__s, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 2, 0);
int	__m_sprintf(char *__s, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 2, 0);

int	__d_snprintf(char *__s, size_t __n, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 3, 0);
int	__f_snprintf(char *__s, size_t __n, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 3, 0);
int	__i_snprintf(char *__s, size_t __n, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 3, 0);
int	__l_snprintf(char *__s, size_t __n, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 3, 0);
int	__m_snprintf(char *__s, size_t __n, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 3, 0);

int	__d_vfscanf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(scanf, 2, 0);
int	__f_vfscanf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(scanf, 2, 0);
int	__i_vfscanf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(scanf, 2, 0);
int	__l_vfscanf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(scanf, 2, 0);
int	__m_vfscanf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(scanf, 2, 0);

#if __SIZEOF_DOUBLE__ == 8
#define FLOAT64 double
#define _asdouble(x) _asfloat64(x)
#elif __SIZEOF_LONG_DOUBLE__ == 8
#define FLOAT64 long double
#endif

#if __SIZEOF_DOUBLE__ == 4
#define FLOAT32 double
#define _asdouble(x) ((double) _asfloat(x))
#elif __SIZEOF_FLOAT__ == 4
#define FLOAT32 float
#elif __SIZEOF_LONG_DOUBLE__ == 4
#define FLOAT32 long double
#endif

#ifdef FLOAT64
FLOAT64
__atod_engine(uint64_t m10, int e10);
#endif

float
__atof_engine(uint32_t m10, int e10);

#ifdef __SIZEOF_INT128__
typedef __uint128_t _u128;
typedef __int128_t _i128;
#define to_u128(x)              (x)
#define from_u128(x)            (x)
#define _u128_to_ld(a)          ((long double) (a))
#define _u128_is_zero(a)        ((a) == 0)
#define _i128_lt_zero(a)        ((_i128) (a) < 0)
#define _u128_plus_64(a,b) ((a) + (b))
#define _u128_plus(a,b) ((a) + (b))
#define _u128_minus(a,b) ((a) - (b))
#define _u128_minus_64(a,b) ((a) - (b))
#define _u128_times_10(a) ((a) * 10)
#define _u128_times_base(a,b)   ((a) * (b))
#define _u128_to_ld(a) ((long double) (a))
#define _u128_oflow(a)	((a) >= (((((_u128) 0xffffffffffffffffULL) << 64) | 0xffffffffffffffffULL) - 9 / 10))
#define _u128_zero	(_u128) 0
#define _u128_lshift(a,b)       ((_u128) (a) << (b))
#define _u128_lshift_64(a,b)    ((_u128) (a) << (b))
#define _u128_rshift(a,b)       ((a) >> (b))
#define _i128_rshift(a,b)       ((_i128) (a) >> (b))
#define _u128_or_64(a,b)        ((a) | (_u128) (b))
#define _u128_and_64(a,b)       ((uint64_t) (a) & (b))
#define _u128_or(a,b)           ((a) | (b))
#define _u128_and(a,b)          ((a) & (b))
#define _u128_eq(a,b)           ((a) == (b))
#define _u128_ge(a,b)           ((a) >= (b))
#define _i128_ge(a,b)           ((_i128)(a) >= (_i128)(b))
#define _u128_lt(a,b)           ((a) < (b))
#define _i128_lt(a,b)           ((_i128)(a) < (_i128)(b))
#define _u128_not(a)            (~(a))
#else
typedef struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint64_t	lo, hi;
#else
    uint64_t	hi, lo;
#endif
} _u128;
#define _u128_zero	(_u128) { 0, 0 }

static inline _u128 to_u128(uint64_t x)
{
    _u128 a = { .hi = 0, .lo = x };
    return a;
}

static inline uint64_t from_u128(_u128 a)
{
    return a.lo;
}

static inline long double
_u128_to_ld(_u128 a)
{
    return (long double) a.hi * ((long double) (1LL << 32) * (long double) (1LL << 32)) + (long double) a.lo;
}

static inline bool
_u128_is_zero(_u128 a)
{
    return a.hi == 0 && a.lo == 0;
}

static inline bool
_i128_lt_zero(_u128 a)
{
    return (int64_t) a.hi < 0;
}

static inline bool
_u128_eq(_u128 a, _u128 b)
{
    return (a.hi == b.hi) && (a.lo == b.lo);
}

static inline bool
_u128_lt(_u128 a, _u128 b)
{
    if (a.hi == b.hi)
        return a.lo < b.lo;
    return a.hi < b.hi;
}

static inline bool
_i128_lt(_u128 a, _u128 b)
{
    if (a.hi == b.hi) {
        if ((int64_t) a.hi < 0)
            return a.lo > b.lo;
        else
            return a.lo < b.lo;
    }
    return (int64_t) a.hi < (int64_t) b.hi;
}

static inline bool
_u128_ge(_u128 a, _u128 b)
{
    if (a.hi == b.hi)
        return a.lo >= b.lo;
    return a.hi >= b.hi;
}

static inline bool
_i128_ge(_u128 a, _u128 b)
{
    if (a.hi == b.hi) {
        if ((int64_t) a.hi < 0)
            return a.lo <= b.lo;
        else
            return a.lo >= b.lo;
    }
    return (int64_t) a.hi >= (int64_t) b.hi;
}

static inline _u128
_u128_plus_64(_u128 a, uint64_t b)
{
    _u128 v;

    v.lo = a.lo + b;
    v.hi = a.hi;
    if (v.lo < a.lo)
	v.hi++;
    return v;
}

static inline _u128
_u128_plus(_u128 a, _u128 b)
{
    _u128 v;

    v.lo = a.lo + b.lo;
    v.hi = a.hi + b.hi;
    if (v.lo < a.lo)
	v.hi++;
    return v;
}

static inline _u128
_u128_minus_64(_u128 a, uint64_t b)
{
    _u128 v;

    v.lo = a.lo - b;
    v.hi = a.hi;
    if (v.lo > a.lo)
	v.hi--;
    return v;
}

static inline _u128
_u128_minus(_u128 a, _u128 b)
{
    _u128 v;

    v.lo = a.lo - b.lo;
    v.hi = a.hi - b.hi;
    if (v.lo > a.lo)
	v.hi--;
    return v;
}

static inline _u128
_u128_lshift(_u128 a, int amt)
{
    _u128	v;

    if (amt == 0) {
        v = a;
    } else if (amt < 64) {
        v.lo = a.lo << amt;
        v.hi = (a.lo >> (64 - amt)) | (a.hi << amt);
    } else {
        v.lo = 0;
        v.hi = a.lo << (amt - 64);
    }
    return v;
}

static inline _u128
_u128_lshift_64(uint64_t a, int amt)
{
    _u128	v;

    if (amt == 0) {
        v.lo = a;
        v.hi = 0;
    } else if (amt < 64) {
        v.lo = a << amt;
        v.hi = (a >> (64 - amt));
    } else {
        v.lo = 0;
        v.hi = a << (amt - 64);
    }
    return v;
}

static inline _u128
_u128_rshift(_u128 a, int amt)
{
    _u128	v;

    if (amt == 0) {
        v = a;
    } else if (amt < 64) {
        v.lo = (a.hi << (64 - amt)) | (a.lo >> amt);
        v.hi = a.hi >> amt;
    } else {
        v.hi = 0;
        v.lo = a.hi >> (amt - 64);
    }
    return v;
}

static inline _u128
_u128_and(_u128 a, _u128 b)
{
    _u128       v;

    v.hi = a.hi & b.hi;
    v.lo = a.lo & b.lo;
    return v;
}

static inline uint64_t
_u128_and_64(_u128 a, uint64_t b)
{
    return a.lo & b;
}

static inline _u128
_u128_or(_u128 a, _u128 b)
{
    _u128       v;

    v.lo = a.lo | b.lo;
    v.hi = a.hi | b.hi;
    return v;
}

static inline _u128
_u128_or_64(_u128 a, uint64_t b)
{
    _u128       v;

    v.lo = a.lo | b;
    v.hi = a.hi;
    return v;
}

static inline _u128
_u128_not(_u128 a)
{
    _u128       v;

    v.lo = ~a.lo;
    v.hi = ~a.hi;
    return v;
}

static inline _u128
_u128_times_10(_u128 a)
{
    return _u128_plus(_u128_lshift(a, 3), _u128_lshift(a, 1));
}

static inline _u128
_u128_times_base(_u128 a, int base)
{
    if (base == 10)
        return _u128_times_10(a);
    return _u128_lshift(a, 4);
}

static inline bool
_u128_oflow(_u128 a)
{
    return a.hi >= (0xffffffffffffffffULL - 9) / 10;
}
#endif

#if __SIZEOF_LONG_DOUBLE__ > 8
static inline _u128
asuintld(long double f)
{
    union {
        long double     f;
        _u128           i;
    } v;
    _u128       i;

    v.f = f;
    i = v.i;
#if defined(__IEEE_BIG_ENDIAN) && __SIZEOF_LONG_DOUBLE__ != 16
    i = _u128_rshift(i, (16 - __SIZEOF_LONG_DOUBLE__) * 8);
#endif
    return i;
}

static inline long double
aslongdouble(_u128 i)
{
    union {
        long double     f;
        _u128           i;
    } v;

#if defined(__IEEE_BIG_ENDIAN) && __SIZEOF_LONG_DOUBLE__ != 16
    i = _u128_lshift(i, (16 - __SIZEOF_LONG_DOUBLE__) * 8);
#endif
    v.i = i;
    return v.f;
}
#elif __SIZEOF_LONG_DOUBLE__ == 8
static inline uint64_t
asuintld(long double f)
{
    union {
        long double     f;
        uint64_t        i;
    } v;

    v.f = f;
    return v.i;
}

static inline long double
aslongdouble(uint64_t i)
{
    union {
        long double     f;
        uint64_t        i;
    } v;

    v.i = i;
    return v.f;
}
#elif __SIZEOF_LONG_DOUBLE__ == 4
static inline uint32_t
asuintld(long double f)
{
    union {
        long double     f;
        uint32_t        i;
    } v;

    v.f = f;
    return v.i;
}

static inline long double
aslongdouble(uint32_t i)
{
    union {
        long double     f;
        uint32_t        i;
    } v;

    v.i = i;
    return v.f;
}
#endif

static inline bool
_u128_gt(_u128 a, _u128 b)
{
    return _u128_lt(b, a);
}

long double
__atold_engine(_u128 m10, int e10);

static inline uint16_t
__non_atomic_exchange_ungetc(__ungetc_t *p, __ungetc_t v)
{
	__ungetc_t e = *p;
	*p = v;
	return e;
}

static inline bool
__non_atomic_compare_exchange_ungetc(__ungetc_t *p, __ungetc_t d, __ungetc_t v)
{
	if (*p != d)
		return false;
	*p = v;
	return true;
}

static inline uint16_t
__non_atomic_load_ungetc(const volatile __ungetc_t *p)
{
        return *p;
}

#if defined(__LONG_DOUBLE_128__) && defined(__strong_reference)
#if defined(__GNUCLIKE_PRAGMA_DIAGNOSTIC) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
#define __ieee128_reference(a,b) __strong_reference(a,b)
#else
#define __ieee128_reference(a,b)
#endif

#ifdef __ATOMIC_UNGETC

#if __PICOLIBC_UNGETC_SIZE == 4 && defined (__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
#define PICOLIBC_HAVE_SYNC_COMPARE_AND_SWAP
#endif

#if __PICOLIBC_UNGETC_SIZE == 2 && defined (__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2)
#define PICOLIBC_HAVE_SYNC_COMPARE_AND_SWAP
#endif

#ifdef PICOLIBC_HAVE_SYNC_COMPARE_AND_SWAP

/* Use built-in atomic functions if they exist */
#include <stdatomic.h>
static inline bool
__atomic_compare_exchange_ungetc(__ungetc_t *p, __ungetc_t d, __ungetc_t v)
{
	_Atomic __ungetc_t *pa = (_Atomic __ungetc_t *) p;
        return atomic_compare_exchange_strong(pa, &d, v);
}

static inline __ungetc_t
__atomic_exchange_ungetc(__ungetc_t *p, __ungetc_t v)
{
	_Atomic __ungetc_t *pa = (_Atomic __ungetc_t *) p;
	return atomic_exchange_explicit(pa, v, memory_order_relaxed);
}

static inline __ungetc_t
__atomic_load_ungetc(const volatile __ungetc_t *p)
{
	_Atomic __ungetc_t *pa = (_Atomic __ungetc_t *) p;
        return atomic_load(pa);
}
#else

bool
__atomic_compare_exchange_ungetc(__ungetc_t *p, __ungetc_t d, __ungetc_t v);

__ungetc_t
__atomic_exchange_ungetc(__ungetc_t *p, __ungetc_t v);

__ungetc_t
__atomic_load_ungetc(const volatile __ungetc_t *p);

__ungetc_t
__picolibc_non_atomic_load_ungetc(const volatile __ungetc_t *p);

__ungetc_t
__picolibc_non_atomic_exchange_ungetc(__ungetc_t *p, __ungetc_t v);

bool
__picolibc_non_atomic_compare_exchange_ungetc(__ungetc_t *p,
                                              __ungetc_t d, __ungetc_t v);

#endif /* PICOLIBC_HAVE_SYNC_COMPARE_AND_SWAP */

#else

#define __atomic_compare_exchange_ungetc(p,d,v) __non_atomic_compare_exchange_ungetc(p,d,v)

#define __atomic_exchange_ungetc(p,v) __non_atomic_exchange_ungetc(p,v)

#define __atomic_load_ungetc(p) (*(p))

#endif /* __ATOMIC_UNGETC */

/*
 * This operates like _tolower on upper case letters, but also works
 * correctly on lower case letters.
 */
#define TOLOWER(c)      ((c) | ('a' - 'A'))

/*
 * Convert a single character to the value of the digit for any
 * character 0 .. 9, A .. Z or a .. z
 *
 * Characters out of these ranges will return a value above 36
 *
 * Work in unsigned int type to avoid extra instructions required for
 * unsigned char folding. The only time this matters is when
 * subtracting '0' from values below '0', which results in very large
 * unsigned values.
 */

static inline unsigned int
digit_to_val(unsigned int c)
{
    /*
     * Convert letters with some tricky code.
     *
     * TOLOWER(c-1) maps characters as follows (Skipping values not
     * greater than '9' (0x39), as those are skipped by the 'if'):
     *
     * Minus 1, bitwise-OR ('a' - 'A') (0x20):
     *
     *             0x3a..0x40 -> 0x39..0x3f
     * 0x41..0x60, 0x61..0x80 -> 0x60..0x7f
     * 0x81..0xa0, 0xa1..0xc0 -> 0xa0..0xbf
     * 0xc1..0xe0, 0xe1..0x00 -> 0xe0..0xff
     *
     * Plus '0' (0x30), minus 'a') (0x61), plus 11 (0xb), for
     * a total of minus 0x26:
     *
     *             0x3a..0x40 -> 0x39..0x3f -> 0x13..0x19
     * 0x41..0x60, 0x61..0x80 -> 0x60..0x7f -> 0x3a..0x59
     * 0x81..0xa0, 0xa1..0xc0 -> 0xa0..0xbf -> 0x7a..0x99
     * 0xc1..0xe0, 0xe1..0x00 -> 0xe0..0xff -> 0xba..0xd9
     */

    if (c > '9') {

        /*
         * For the letters, we want TOLOWER(c) - 'a' + 10, but that
         * would map both '@' and '`' to 9.
         *
         * To work around this, subtract 1 before the bitwise-or so
         * that '@' (0x40) gets mapped down to 0x3f (0x3f | 0x20)
         * while '`' (0x60) gets mapped up to 0x7f (0x5f | 0x20),
         * moving them away from the letters (which end up in the
         * range 0x60..0x79). Then add the 1 back in when subtracting
         * 'a' and adding 10.
         *
         * Add in '0' so that it can get subtracted out in the common
         * code (c -= '0') below, avoiding an else clause.
         */

        c = TOLOWER(c-1) + ('0' - 'a' + 11);
    }

    /*
     * Now, include the range from NUL (0x00) through '9' (0x39)
     *
     * Minus '0' (0x30):
     *
     * 0x00..0x2f                                         ->-0x30..-0x01
     * 0x30..0x39                                         -> 0x00..0x09 *
     *             0x3a..0x40 -> 0x39..0x3f -> 0x13..0x19 ->-0x1d..-0x17
     * 0x41..0x60, 0x61..0x80 -> 0x60..0x7f -> 0x3a..0x59 -> 0x0a..0x29 *
     * 0x81..0xa0, 0xa1..0xc0 -> 0xa0..0xbf -> 0x7a..0x99 -> 0x4a..0x69
     * 0xc1..0xe0, 0xe1..0x00 -> 0xe0..0xff -> 0xba..0xd9 -> 0x8a..0xa9
     *
     * The first starred row has the digits '0'..'9', while the second
     * starts with the letters 'A'..'Z' and 'a'..'z'. All of the other
     * rows end up with values above any allowed conversion base
     */

    c -= '0';
    return c;
}

#endif /* _STDIO_PRIVATE_H_ */
