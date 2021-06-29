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

#include <stdio.h>
#include <stdbool.h>
#include <sys/lock.h>

/* values for PRINTF_LEVEL */
#define PRINTF_STD 1
#define PRINTF_FLT 2

/* values for SCANF_LEVEL */
#define SCANF_STD 1
#define SCANF_FLT 2

/* This is OR'd into the char stored in unget to make it non-zero to
 * flag the unget buffer as being full
 */
#define UNGETC_MARK	0x0100

struct __file_str {
	struct __file file;	/* main file struct */
	char	*buf;		/* buffer pointer */
	int	len;		/* characters written so far */
	int	size;		/* size of buffer */
};

int
__file_str_get(FILE *stream);

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
			.get = __file_str_get	\
		},				\
		.buf = (char *) (_s)		\
	}

#define FDEV_SETUP_STRING_WRITE(_s, _size) {	\
		.file = {			\
			.flags = __SWR,		\
			.put = __file_str_put	\
		},				\
		.buf = (_s),			\
		.size = (_size),		\
		.len = 0			\
	}

#define FDEV_SETUP_STRING_ALLOC() {		\
		.file = {			\
			.flags = __SWR,		\
			.put = __file_str_put_alloc	\
		},				\
		.buf = NULL,			\
		.size = 0,			\
		.len = 0			\
	}

#ifdef POSIX_IO

struct __file_posix {
	struct __file_close cfile;
	int	fd;
	char	*write_buf;
	int	write_len;
	char	*read_buf;
	int	read_len;
	int	read_off;
#ifndef __SINGLE_THREAD__
	_LOCK_T lock;
#endif
};

static inline void __posix_lock_init(FILE *f) {
	(void) f;
	__lock_init(((struct __file_posix *) f)->lock);
}

static inline void __posix_lock_close(FILE *f) {
	(void) f;
	__lock_close(((struct __file_posix *) f)->lock);
}

static inline void __posix_lock(FILE *f) {
	(void) f;
	__lock_acquire(((struct __file_posix *) f)->lock);
}

static inline void __posix_unlock(FILE *f) {
	(void) f;
	__lock_release(((struct __file_posix *) f)->lock);
}

int	__d_vfprintf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(printf, 2, 0);
int	__f_vfprintf(FILE *__stream, const char *__fmt, va_list __ap) __FORMAT_ATTRIBUTE__(printf, 2, 0);

int
__posix_sflags (const char *mode, int *optr);

int
__posix_flush(FILE *f);

int
__posix_putc(char c, FILE *f);

int
__posix_getc(FILE *f);

int
__posix_close(FILE *f);

#endif

int	__d_sprintf(char *__s, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 2, 0);
int	__f_sprintf(char *__s, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 2, 0);
int	__d_snprintf(char *__s, size_t __n, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 3, 0);
int	__f_snprintf(char *__s, size_t __n, const char *__fmt, ...) __FORMAT_ATTRIBUTE__(printf, 3, 0);

double
__atod_engine(uint64_t m10, int e10);

float
__atof_engine(uint32_t m10, int e10);

#ifdef __SIZEOF_INT128__
typedef __uint128_t _u128;
#define to_u128(x)              (x)
#define from_u128(x)            (x)
#define _u128_to_ld(a)          ((long double) (a))
#define _u128_is_zero(a)        ((a) == 0)
#define _u128_plus_64(a,b) ((a) + (b))
#define _u128_plus(a,b) ((a) + (b))
#define _u128_times_10(a) ((a) * 10)
#define _u128_times_base(a,b)   ((a) * (b))
#define _u128_to_ld(a) ((long double) (a))
#define _u128_oflow(a)	((a) >= (((((_u128) 0xffffffffffffffffULL) << 64) | 0xffffffffffffffffULL) - 9 / 10))
#define _u128_zero	(_u128) 0
#else
typedef struct {
    uint64_t	hi, lo;
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
_u128_lshift(_u128 a, int amt)
{
    _u128	v;

    v.lo = a.lo << amt;
    v.hi = (a.lo >> (64 - amt)) | (a.hi << amt);
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

#ifdef ATOMIC_UNGETC

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
	return atomic_compare_exchange_weak(pa, &d, v);
}
static inline __ungetc_t
__atomic_exchange_ungetc(__ungetc_t *p, __ungetc_t v)
{
	_Atomic __ungetc_t *pa = (_Atomic __ungetc_t *) p;
	return atomic_exchange_explicit(pa, v, memory_order_relaxed);
}

#else

bool
__atomic_compare_exchange_ungetc(__ungetc_t *p, __ungetc_t d, __ungetc_t v);

__ungetc_t
__atomic_exchange_ungetc(__ungetc_t *p, __ungetc_t v);

#endif /* PICOLIBC_HAVE_SYNC_COMPARE_AND_SWAP */

#else

#define __atomic_compare_exchange_ungetc(p,d,v) __non_atomic_compare_exchange_ungetc(p,d,v)

#define __atomic_exchange_ungetc(p,v) __non_atomic_exchange_ungetc(p,v)

#endif /* ATOMIC_UNGETC */
