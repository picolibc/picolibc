/* Copyright (c) 2002, Alexander Popov (sasho@vip.bg)
   Copyright (c) 2002,2004,2005 Joerg Wunsch
   Copyright (c) 2005, Helmut Wallner
   Copyright (c) 2007, Dmitry Xmelkov
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

/* From: Id: printf_p_new.c,v 1.1.1.9 2002/10/15 20:10:28 joerg_wunsch Exp */
/* $Id: vfprintf.c 2191 2010-11-05 13:45:57Z arcanum $ */

#define _DEFAULT_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stdio_private.h"
#include "../../libm/common/math_config.h"

#ifdef WIDE_CHARS
#include <wchar.h>
#define CHAR wchar_t
#define UCHAR wchar_t
#define vfprintf vfwprintf
#define PRINTF_LEVEL PRINTF_DBL
#else
#define CHAR char
#define UCHAR unsigned char
#endif

/*
 * This file can be compiled into more than one flavour:
 *
 *  PRINTF_MIN: limited integer-only support with option for long long
 *
 *  PRINTF_STD: full integer support with options for positional
 *              params and long long
 *
 *  PRINTF_FLT: full support
 */

#ifndef PRINTF_LEVEL
#  define PRINTF_LEVEL PRINTF_DBL
#  ifndef FORMAT_DEFAULT_DOUBLE
#    define vfprintf __d_vfprintf
#  endif
#endif

/*
 * Compute which features are required. This should match the _HAS
 * values computed in stdio.h
 */

#if PRINTF_LEVEL == PRINTF_MIN
# define _NEED_IO_SHRINK
# if defined(_WANT_MINIMAL_IO_LONG_LONG) && __SIZEOF_LONG_LONG > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# ifdef _WANT_IO_C99_FORMATS
#  define _NEED_IO_C99_FORMATS
# endif
#elif PRINTF_LEVEL == PRINTF_STD
# if defined(_WANT_IO_LONG_LONG) && __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# ifdef _WANT_IO_POS_ARGS
#  define _NEED_IO_POS_ARGS
# endif
# ifdef _WANT_IO_C99_FORMATS
#  define _NEED_IO_C99_FORMATS
# endif
# ifdef _WANT_IO_PERCENT_B
#  define _NEED_IO_PERCENT_B
# endif
#elif PRINTF_LEVEL == PRINTF_FLT
# if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# define _NEED_IO_POS_ARGS
# define _NEED_IO_C99_FORMATS
# ifdef _WANT_IO_PERCENT_B
#  define _NEED_IO_PERCENT_B
# endif
# define _NEED_IO_FLOAT
#elif PRINTF_LEVEL == PRINTF_DBL
# if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# define _NEED_IO_POS_ARGS
# define _NEED_IO_C99_FORMATS
# ifdef _WANT_IO_PERCENT_B
#  define _NEED_IO_PERCENT_B
# endif
# define _NEED_IO_DOUBLE
# if defined(_WANT_IO_LONG_DOUBLE) && __SIZEOF_LONG_DOUBLE__ > __SIZEOF_DOUBLE__
#  define _NEED_IO_LONG_DOUBLE
# endif
#else
# error invalid PRINTF_LEVEL
#endif

#ifdef _NEED_IO_FLOAT
static inline float
printf_float_get(uint32_t u)
{
    return asfloat(u);
}

#define PRINTF_FLOAT_ARG(ap) (printf_float_get(va_arg(ap, uint32_t)))
typedef float FLOAT;
typedef int32_t INTFLOAT;
typedef uint32_t UINTFLOAT;
#define ASUINT(a) asuint(a)
#define EXP_BIAS 127

#define dtoa ftoa
#define DTOA_MINUS FTOA_MINUS
#define DTOA_ZERO  FTOA_ZERO
#define DTOA_INF   FTOA_INF
#define DTOA_NAN   FTOA_NAN
#define DTOA_CARRY FTOA_CARRY
#define DTOA_MAX_DIG FTOA_MAX_DIG
#define __dtoa_engine(x,dtoa,dig,f,dec) __ftoa_engine(x,dtoa,dig,f,dec)
#include "ftoa_engine.h"

#elif defined(_NEED_IO_DOUBLE)

#define PRINTF_FLOAT_ARG(ap) va_arg(ap, double)
typedef double FLOAT;
typedef uint64_t UINTFLOAT;
typedef int64_t INTFLOAT;
#define ASUINT(a) asuint64(a)
#define EXP_BIAS 1023
#include "dtoa_engine.h"

#endif

#ifdef _NEED_IO_FLOAT
#define SKIP_FLOAT_ARG(flags, ap) (void) va_arg(ap, uint32_t)
#else
#if __SIZEOF_LONG_DOUBLE__ > __SIZEOF_DOUBLE__
#define SKIP_FLOAT_ARG(flags, ap) do {                                  \
        if ((flags & (FL_LONG | FL_REPD_TYPE)) == (FL_LONG | FL_REPD_TYPE)) \
            (void) va_arg(ap, long double);                             \
        else                                                            \
            (void) va_arg(ap, double);                                  \
    } while(0)
#else
#define SKIP_FLOAT_ARG(flags, ap) (void) va_arg(ap, double)
#endif
#endif

#ifdef _NEED_IO_LONG_DOUBLE
# include "ldtoa_engine.h"
#endif

#ifdef _NEED_IO_LONG_LONG
typedef unsigned long long ultoa_unsigned_t;
typedef long long ultoa_signed_t;
#define SIZEOF_ULTOA __SIZEOF_LONG_LONG__
#define arg_to_t(ap, flags, _s_, _result_)              \
    if ((flags) & FL_LONG) {                            \
        if ((flags) & FL_REPD_TYPE)                     \
            (_result_) = va_arg(ap, _s_ long long);     \
        else                                            \
            (_result_) = va_arg(ap, _s_ long);          \
    } else {                                            \
        (_result_) = va_arg(ap, _s_ int);               \
        if ((flags) & FL_SHORT) {                       \
            if ((flags) & FL_REPD_TYPE)                 \
                (_result_) = (_s_ char) (_result_);     \
            else                                        \
                (_result_) = (_s_ short) (_result_);    \
        }                                               \
    }
#else
typedef unsigned long ultoa_unsigned_t;
typedef long ultoa_signed_t;
#define SIZEOF_ULTOA __SIZEOF_LONG__
#define arg_to_t(ap, flags, _s_, _result_)              \
    if ((flags) & FL_LONG) {                            \
        if ((flags) & FL_REPD_TYPE)                     \
            (_result_) = (_s_ long) va_arg(ap, _s_ long long);\
        else                                            \
            (_result_) = va_arg(ap, _s_ long);          \
    } else {                                            \
        (_result_) = va_arg(ap, _s_ int);               \
        if ((flags) & FL_SHORT) {                       \
            if ((flags) & FL_REPD_TYPE)                 \
                (_result_) = (_s_ char) (_result_);     \
            else                                        \
                (_result_) = (_s_ short) (_result_);    \
        }                                               \
    }
#endif

#if SIZEOF_ULTOA <= 4
#ifdef _NEED_IO_PERCENT_B
#define PRINTF_BUF_SIZE 32
#else
#define PRINTF_BUF_SIZE 11
#endif
#else
#ifdef _NEED_IO_PERCENT_B
#define PRINTF_BUF_SIZE 64
#else
#define PRINTF_BUF_SIZE 22
#endif
#endif

// At the call site the address of the result_var is taken (e.g. "&ap")
// That way, it's clear that these macros *will* modify that variable
#define arg_to_unsigned(ap, flags, result_var) arg_to_t(ap, flags, unsigned, result_var)
#define arg_to_signed(ap, flags, result_var) arg_to_t(ap, flags, signed, result_var)

#include "ultoa_invert.c"

/* Order is relevant here and matches order in format string */

#ifdef _NEED_IO_SHRINK
#define FL_ZFILL	0x0000
#define FL_PLUS		0x0000
#define FL_SPACE	0x0000
#define FL_LPAD		0x0000
#else
#define FL_ZFILL	0x0001
#define FL_PLUS		0x0002
#define FL_SPACE	0x0004
#define FL_LPAD		0x0008
#endif /* else _NEED_IO_SHRINK */
#define FL_ALT		0x0010

#define FL_WIDTH	0x0020
#define FL_PREC		0x0040

#define FL_LONG		0x0080
#define FL_SHORT	0x0100
#define FL_REPD_TYPE	0x0200

#define FL_NEGATIVE	0x0400

#ifdef _NEED_IO_C99_FORMATS
#define FL_FLTHEX       0x0800
#endif
#define FL_FLTEXP	0x1000
#define	FL_FLTFIX	0x2000

#ifdef _NEED_IO_POS_ARGS

typedef struct {
    va_list     ap;
} my_va_list;

/*
 * Repeatedly scan the format string finding argument position values
 * and types to slowly walk the argument vector until it points at the
 * target_argno so that the outer printf code can then extract it.
 */
static void
skip_to_arg(const CHAR *fmt_orig, my_va_list *ap, int target_argno)
{
    unsigned c;		/* holds a char from the format string */
    uint16_t flags;
    int current_argno = 1;
    int argno;
    int width;
    const CHAR *fmt = fmt_orig;

    while (current_argno < target_argno) {
        for (;;) {
            c = *fmt++;
            if (!c) return;
            if (c == '%') {
                c = *fmt++;
                if (c != '%') break;
            }
        }
        flags = 0;
        width = 0;
        argno = 0;

        /*
         * Scan the format string until we hit current_argno. This can
         * either be a value, width or precision field.
         */
        do {
	    if (flags < FL_WIDTH) {
		switch (c) {
		  case '0':
		    continue;
		  case '+':
		  case ' ':
		    continue;
		  case '-':
		    continue;
		  case '#':
		    continue;
                  case '\'':
                    continue;
		}
	    }

	    if (flags < FL_LONG) {
		if (c >= '0' && c <= '9') {
		    c -= '0';
                    width = 10 * width + c;
                    flags |= FL_WIDTH;
		    continue;
		}
                if (c == '$') {
                    /*
                     * If we've already seen the value position, any
                     * other positions will be either width or
                     * precisions. We can handle those in the same
                     * fashion as they're both 'int' type
                     */
                    if (argno) {
                        if (width == current_argno) {
                            c = 'c';
                            argno = width;
                            break;
                        }
                    }
                    else
                        argno = width;
                    width = 0;
                    continue;
                }

		if (c == '*') {
                    width = 0;
		    continue;
                }

		if (c == '.') {
                    width = 0;
		    continue;
                }
	    }

	    if (c == 'l') {
		if (flags & FL_LONG)
		    flags |= FL_REPD_TYPE;
		flags |= FL_LONG;
		continue;
	    }

	    if (c == 'h') {
		if (flags & FL_SHORT)
		    flags |= FL_REPD_TYPE;
		flags |= FL_SHORT;
		continue;
	    }

            /* alias for 'll' */
            if (c == 'L') {
                flags |= FL_REPD_TYPE;
		flags |= FL_LONG;
		continue;
            }

#ifdef _NEED_IO_C99_FORMATS

#define CHECK_LONGLONG(type) else if (sizeof(type) == sizeof(long long)) flags |= FL_LONG|FL_REPD_TYPE;

#define CHECK_INT_SIZE(letter, type)			\
	    if (c == letter) {				\
		if (sizeof(type) == sizeof(int))	\
		    ;                                   \
		else if (sizeof(type) == sizeof(long))	\
		    flags |= FL_LONG;                   \
                CHECK_LONGLONG(type)                    \
		else if (sizeof(type) == sizeof(short))	\
		    flags |= FL_SHORT;			\
                continue;                               \
	    }

	    CHECK_INT_SIZE('j', intmax_t);
	    CHECK_INT_SIZE('z', size_t);
	    CHECK_INT_SIZE('t', ptrdiff_t);
#endif
	    break;
	} while ( (c = *fmt++) != 0);
        if (argno == 0)
            break;
        if (argno == current_argno) {
            if ((TOLOW(c) >= 'e' && TOLOW(c) <= 'g')
#ifdef _NEED_IO_C99_FORMATS
                || TOLOW(c) == 'a'
#endif
                ) {
                SKIP_FLOAT_ARG(flags, ap->ap);
            } else if (c == 'c') {
                (void) va_arg (ap->ap, int);
            } else if (c == 's') {
                (void) va_arg (ap->ap, char *);
            } else if (c == 'd' || c == 'i') {
                ultoa_signed_t x_s;
                arg_to_signed(ap->ap, flags, x_s);
            } else {
                ultoa_unsigned_t x;
                arg_to_unsigned(ap->ap, flags, x);
            }
            ++current_argno;
            fmt = fmt_orig;
        }
    }
}
#endif

int vfprintf (FILE * stream, const CHAR *fmt, va_list ap_orig)
{
    unsigned c;		/* holds a char from the format string */
    uint16_t flags;
    int width;
    int prec;
#ifdef _NEED_IO_POS_ARGS
    int argno;
    my_va_list my_ap;
    const CHAR *fmt_orig = fmt;
#define ap my_ap.ap
#else
#define ap ap_orig
#endif
    union {
	char __buf[PRINTF_BUF_SIZE];	/* size for -1 in smallest base, without '\0'	*/
#if PRINTF_LEVEL >= PRINTF_FLT
	struct dtoa __dtoa;
#ifdef _NEED_IO_LONG_DOUBLE
        struct ldtoa __ldtoa;
#endif
#endif
    } u;
    const char * pnt;
#ifndef _NEED_IO_SHRINK
    size_t size;
#endif

#define buf	(u.__buf)
#define _dtoa	(u.__dtoa)

    int stream_len = 0;

#ifndef my_putc
    int (*put)(char, FILE *) = stream->put;
#define my_putc(c, stream) do { ++stream_len; if (put(c, stream) < 0) goto fail; } while(0)
#endif

    if ((stream->flags & __SWR) == 0)
	return EOF;

#ifdef _NEED_IO_POS_ARGS
    va_copy(ap, ap_orig);
#endif

    for (;;) {

	for (;;) {
	    c = *fmt++;
	    if (!c) goto ret;
	    if (c == '%') {
		c = *fmt++;
		if (c != '%') break;
	    }
	    my_putc (c, stream);
	}

	flags = 0;
	width = 0;
	prec = 0;
#ifdef _NEED_IO_POS_ARGS
        argno = 0;
#endif

	do {
	    if (flags < FL_WIDTH) {
		switch (c) {
		  case '0':
		    flags |= FL_ZFILL;
		    continue;
		  case '+':
		    flags |= FL_PLUS;
		    __PICOLIBC_FALLTHROUGH;
		  case ' ':
		    flags |= FL_SPACE;
		    continue;
		  case '-':
		    flags |= FL_LPAD;
		    continue;
		  case '#':
		    flags |= FL_ALT;
		    continue;
                  case '\'':
                    /*
                     * C/POSIX locale has an empty thousands_sep
                     * value, so we can just ignore this character
                     */
                    continue;
		}
	    }

	    if (flags < FL_LONG) {
		if (c >= '0' && c <= '9') {
#ifndef _NEED_IO_SHRINK
		    c -= '0';
		    if (flags & FL_PREC) {
			prec = 10*prec + c;
			continue;
		    }
		    width = 10*width + c;
		    flags |= FL_WIDTH;
#endif
		    continue;
		}
		if (c == '*') {
#ifdef _NEED_IO_POS_ARGS
                    /*
                     * Positional args must be used together, so wait
                     * for the value to appear before dealing with
                     * width and precision fields
                     */
                    if (argno)
                        continue;
#endif
		    if (flags & FL_PREC) {
			prec = va_arg(ap, int);
#ifdef _NEED_IO_SHRINK
                        (void) prec;
#endif
		    } else {
			width = va_arg(ap, int);
			flags |= FL_WIDTH;
#ifdef _NEED_IO_SHRINK
                        (void) width;
#else
			if (width < 0) {
			    width = -width;
			    flags |= FL_LPAD;
			}
#endif
		    }
		    continue;
		}
		if (c == '.') {
		    if (flags & FL_PREC)
			goto ret;
		    flags |= FL_PREC;
		    continue;
		}
#ifdef _NEED_IO_POS_ARGS
                /* Check for positional args */
                if (c == '$') {
                    /* Check if we've already got the arg position and
                     * are adding width or precision fields
                     */
                    if (argno) {
                        va_end(ap);
                        va_copy(ap, ap_orig);
                        skip_to_arg(fmt_orig, &my_ap, (flags & FL_PREC) ? prec : width);
                        if (flags & FL_PREC)
                            prec = va_arg(ap, int);
                        else
                            width = va_arg(ap, int);
                    } else {
                        argno = width;
                        flags = 0;
                        width = 0;
                        prec = 0;
                    }
                    continue;
                }
#endif
	    }

	    if (c == 'l') {
		if (flags & FL_LONG)
		    flags |= FL_REPD_TYPE;
		flags |= FL_LONG;
		continue;
	    }

	    if (c == 'h') {
		if (flags & FL_SHORT)
		    flags |= FL_REPD_TYPE;
		flags |= FL_SHORT;
		continue;
	    }

            /* alias for 'll' */
            if (c == 'L') {
                flags |= FL_REPD_TYPE;
		flags |= FL_LONG;
		continue;
            }

#define CHECK_LONGLONG(type) else if (sizeof(type) == sizeof(long long)) flags |= FL_LONG|FL_REPD_TYPE;

#ifdef _NEED_IO_C99_FORMATS

#define CHECK_INT_SIZE(letter, type)			\
	    if (c == letter) {				\
		if (sizeof(type) == sizeof(int))	\
		    ;                                   \
		else if (sizeof(type) == sizeof(long))	\
		    flags |= FL_LONG;                   \
                CHECK_LONGLONG(type)                    \
		else if (sizeof(type) == sizeof(short))	\
		    flags |= FL_SHORT;			\
                continue;                               \
	    }

	    CHECK_INT_SIZE('j', intmax_t);
	    CHECK_INT_SIZE('z', size_t);
	    CHECK_INT_SIZE('t', ptrdiff_t);
#endif

	    break;
	} while ( (c = *fmt++) != 0);

#ifdef _NEED_IO_POS_ARGS
        /* Set arg pointers for positional args */
        if (argno) {
            va_end(ap);
            va_copy(ap, ap_orig);
            skip_to_arg(fmt_orig, &my_ap, argno);
        }
#endif

#ifndef _NEED_IO_SHRINK
	/* This can happen only when prec is set via a '*'
	 * specifier, in which case it works as if no precision
	 * was specified. Set the precision to zero and clear the
	 * flag.
	 */
	if (prec < 0) {
	    prec = 0;
	    flags &= ~FL_PREC;
	}
#endif

	/* Only a format character is valid.	*/

#if	'F' != 'E'+1  ||  'G' != 'F'+1  ||  'f' != 'e'+1  ||  'g' != 'f'+1
# error
#endif

#define TOCASE(c)       ((c) - case_convert)

#ifndef _NEED_IO_SHRINK
	if ((TOLOW(c) >= 'e' && TOLOW(c) <= 'g')
#ifdef _NEED_IO_C99_FORMATS
            || TOLOW(c) == 'a'
#endif
            )
        {
#if PRINTF_LEVEL >= PRINTF_FLT
            uint8_t sign;		/* sign character (or 0)	*/
            uint8_t ndigs;		/* number of digits to convert */
            unsigned char case_convert; /* subtract to correct case */
	    int exp;			/* exponent of most significant decimal digit */
	    int n;                      /* total width */
	    uint8_t ndigs_exp;		/* number of digis in exponent */

            /* deal with upper vs lower case */
            case_convert = TOLOW(c) - c;
            c = TOLOW(c);

#ifdef _NEED_IO_LONG_DOUBLE
            if ((flags & (FL_LONG | FL_REPD_TYPE)) == (FL_LONG | FL_REPD_TYPE))
            {
                long double fval;
                fval = va_arg(ap, long double);

                ndigs = 0;

#ifdef _NEED_IO_C99_FORMATS
                if (c == 'a') {
                    _u128 fi;
                    _u128 s;

                    c = 'p';
                    flags |= FL_FLTEXP | FL_FLTHEX;

                    ndigs = (__LDBL_MANT_DIG__ + 3) / 4;
                    fi = asuint128(fval);


#if __LDBL_MANT_DIG__ == 64
#define LEXP_BIAS       (__LDBL_MAX_EXP__ + 2)
#define LEXP_INF        (__LDBL_MAX_EXP__ - 3)
#define LSIG_BITS       (__LDBL_MANT_DIG__)
#ifdef __m68k__
#define LDENORM_EXP_BIAS 0
#else
#define LDENORM_EXP_BIAS 1
#define LSIG_MSB_INF    _u128_lshift(to_u128(1), __LDBL_MANT_DIG__-1)
#endif
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LEXP_SHIFT       __LDBL_MANT_DIG__
#define LSIGN_SHIFT        79
#endif
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define LEXP_SHIFT       (__LDBL_MANT_DIG__ + 16)
#define LSIGN_SHIFT        (79 + 16)
#endif
#else
#define LDENORM_EXP_BIAS 1
#define LSIGN_SHIFT        127
#define LEXP_BIAS       (__LDBL_MAX_EXP__ - 1)
#define LEXP_INF        (__LDBL_MAX_EXP__)
#define LSIG_MSB        _u128_lshift(to_u128(1), __LDBL_MANT_DIG__-1)
#define LSIG_BITS       (__LDBL_MANT_DIG__ - 1)
#define LEXP_SHIFT      (__LDBL_MANT_DIG__ - 1)
#endif
#define LEXP_MASK        ((__LDBL_MAX_EXP__ - 1) + __LDBL_MAX_EXP__)
#define LSIG_SHIFT       0
#define LSIG_MASK        _u128_minus_64(_u128_lshift(to_u128(1), LSIG_BITS), 1)

                    exp = _u128_and_64(_u128_rshift(fi, LEXP_SHIFT), LEXP_MASK);
                    _dtoa.flags = 0;
                    if (_u128_and_64(_u128_rshift(fi, LSIGN_SHIFT), 1))
                        _dtoa.flags = DTOA_MINUS;
                    s = fi = _u128_lshift(_u128_and(fi, LSIG_MASK), LSIG_SHIFT);
                    if (!_u128_is_zero(s) || exp != 0) {
                        if (exp == 0)
                            exp = LDENORM_EXP_BIAS;
                        else
                        {
#ifdef LSIG_MSB
                            s = _u128_or(s, LSIG_MSB);
#endif
                        }
                        exp -= LEXP_BIAS;
                    }

                    if (!(flags & FL_PREC))
                        prec = 0;
                    else if (prec >= (ndigs - 1))
                        prec = ndigs - 1;
                    else {
                        int     bits = ((ndigs - 1) - prec) << 2;
                        _u128   half = _u128_lshift(to_u128(1), bits - 1);
                        _u128   mask = _u128_not(_u128_minus_64(_u128_lshift(half, 1), 1));

                        /* round even */
                        if (_u128_gt(_u128_and(s, _u128_not(mask)), half) || _u128_and_64(_u128_rshift(s, bits), 1) != 0)
                            s = _u128_plus(s, half);

                        s = _u128_and(s, mask);
                    }

                    if (exp == LEXP_INF) {
#ifdef LSIG_MSB_INF
                        if (!_u128_eq(fi, LSIG_MSB_INF))
#else
                        if (!_u128_is_zero(fi))
#endif
                            _dtoa.flags |= DTOA_NAN;
                        else
                            _dtoa.flags |= DTOA_INF;
                    } else {
                        int8_t d;
                        for (d = ndigs - 1; d >= 0; d--) {
                            char dig = _u128_and_64(s, 0xf);
                            s = _u128_rshift(s, 4);
                            if (dig == 0 && d > prec)
                                continue;
                            if (dig <= 9)
                                dig += '0';
                            else
                                dig += TOCASE('a' - 10);
                            _dtoa.digits[d] = dig;
                            if (prec < d)
                                prec = d;
                        }
                    }
                    ndigs_exp = 1;
                } else
#endif /* _NEED_IO_C99_FORMATS */
                {
                    int ndecimal = 0;	        /* digits after decimal (for 'f' format) */
                    bool fmode = false;

                    if (!(flags & FL_PREC))
                        prec = 6;
                    if (c == 'e') {
                        ndigs = prec + 1;
                        flags |= FL_FLTEXP;
                    } else if (c == 'f') {
                        ndigs = DTOA_MAX_DIG;
                        ndecimal = prec;
                        flags |= FL_FLTFIX;
                        fmode = true;
                    } else {
                        c += 'e' - 'g';
                        ndigs = prec;
                        if (ndigs < 1) ndigs = 1;
                    }

                    if (ndigs > DTOA_MAX_DIG)
                        ndigs = DTOA_MAX_DIG;

                    ndigs = __dtoa_engine ((double) fval, &_dtoa, ndigs, fmode, ndecimal);
                    exp = _dtoa.exp;
                    ndigs_exp = 2;
                }
            }
            else
#endif
            {
                FLOAT fval;        /* value to print */

                fval = PRINTF_FLOAT_ARG(ap);

                ndigs = 0;

#ifdef _NEED_IO_C99_FORMATS
                if (c == 'a') {
                    INTFLOAT fi;
                    ultoa_signed_t      s;

                    c = 'p';
                    flags |= FL_FLTEXP | FL_FLTHEX;

#ifdef _NEED_IO_FLOAT
                    ndigs = 7;
#else
                    ndigs = 14;
#endif
                    fi = ASUINT(fval);

                    _dtoa.digits[0] = '0';
#ifdef _NEED_IO_FLOAT
#define EXP_SHIFT       23
#define EXP_MASK        0xff
#define SIG_SHIFT       1
#define SIG_MASK        0x7fffff
#endif
#ifdef _NEED_IO_DOUBLE
#define EXP_SHIFT       52
#define EXP_MASK        0x7ff
#define SIG_SHIFT       0
#define SIG_MASK        0xfffffffffffffLL
#endif
                    exp = ((fi >> EXP_SHIFT) & EXP_MASK);
                    s = (fi & SIG_MASK) << SIG_SHIFT;
                    if (s | exp) {
                        if (!exp)
                            exp = 1;
                        else
                            _dtoa.digits[0] = '1';
                        exp -= EXP_BIAS;
                    }
                    _dtoa.flags = 0;
                    if (fi < 0)
                        _dtoa.flags = DTOA_MINUS;

                    if (!(flags & FL_PREC))
                        prec = 0;
                    else if (prec >= (ndigs - 1))
                        prec = ndigs - 1;
                    else {
                        int                 bits = ((ndigs - 1) - prec) << 2;
                        ultoa_signed_t      half = ((ultoa_signed_t) 1) << (bits - 1);
                        ultoa_signed_t      mask = ~((half << 1) - 1);

                        /* round even */
                        if ((s & ~mask) > half || ((s >> bits) & 1) != 0)
                            s += half;
                        /* special case rounding first digit */
                        if (s > (SIG_MASK << SIG_SHIFT))
                            _dtoa.digits[0]++;
                        s &= mask;
                    }

                    if (exp == EXP_BIAS + 1) {
                        if (s)
                            _dtoa.flags |= DTOA_NAN;
                        else
                            _dtoa.flags |= DTOA_INF;
                    } else {
                        uint8_t d;
                        for (d = ndigs - 1; d; d--) {
                            char dig = s & 0xf;
                            s >>= 4;
                            if (dig == 0 && d > prec)
                                continue;
                            if (dig <= 9)
                                dig += '0';
                            else
                                dig += TOCASE('a' - 10);
                            _dtoa.digits[d] = dig;
                            if (prec < d)
                                prec = d;
                        }
                    }
                    ndigs_exp = 1;
                } else
#endif /* _NEED_IO_C99_FORMATS */
                {
                    int ndecimal = 0;	        /* digits after decimal (for 'f' format) */
                    bool fmode = false;

                    if (!(flags & FL_PREC))
                        prec = 6;
                    if (c == 'e') {
                        ndigs = prec + 1;
                        flags |= FL_FLTEXP;
                    } else if (c == 'f') {
                        ndigs = DTOA_MAX_DIG;
                        ndecimal = prec;
                        flags |= FL_FLTFIX;
                        fmode = true;
                    } else {
                        c += 'e' - 'g';
                        ndigs = prec;
                        if (ndigs < 1) ndigs = 1;
                    }

                    if (ndigs > DTOA_MAX_DIG)
                        ndigs = DTOA_MAX_DIG;

                    ndigs = __dtoa_engine (fval, &_dtoa, ndigs, fmode, ndecimal);
                    exp = _dtoa.exp;
                    ndigs_exp = 2;
                }
            }
	    if (exp < -9 || 9 < exp)
		    ndigs_exp = 2;
	    if (exp < -99 || 99 < exp)
		    ndigs_exp = 3;
#ifdef _NEED_IO_DOUBLE
	    if (exp < -999 || 999 < exp)
		    ndigs_exp = 4;
#ifdef _NEED_IO_LONG_DOUBLE
	    if (exp < -9999 || 9999 < exp)
		    ndigs_exp = 5;
#endif
#endif

	    sign = 0;
	    if (_dtoa.flags & DTOA_MINUS)
		sign = '-';
	    else if (flags & FL_PLUS)
		sign = '+';
	    else if (flags & FL_SPACE)
		sign = ' ';

	    if (_dtoa.flags & (DTOA_NAN | DTOA_INF))
	    {
		ndigs = sign ? 4 : 3;
		if (width > ndigs) {
		    width -= ndigs;
		    if (!(flags & FL_LPAD)) {
			do {
			    my_putc (' ', stream);
			} while (--width);
		    }
		} else {
		    width = 0;
		}
		if (sign)
		    my_putc (sign, stream);
		pnt = "inf";
		if (_dtoa.flags & DTOA_NAN)
		    pnt = "nan";
		while ( (c = *pnt++) )
		    my_putc (TOCASE(c), stream);
	    }
            else
            {

                if (!(flags & (FL_FLTEXP|FL_FLTFIX))) {

                    /* 'g(G)' format */

                    /*
                     * On entry to this block, prec is
                     * the number of digits to display.
                     *
                     * On exit, prec is the number of digits
                     * to display after the decimal point
                     */

                    /* Always show at least one digit */
                    if (prec == 0)
                        prec = 1;

                    /*
                     * Remove trailing zeros. The ryu code can emit them
                     * when rounding to fewer digits than required for
                     * exact output, the imprecise code often emits them
                     */
                    while (ndigs > 0 && _dtoa.digits[ndigs-1] == '0')
                        ndigs--;

                    /* Save requested precision */
                    int req_prec = prec;

                    /* Limit output precision to ndigs unless '#' */
                    if (!(flags & FL_ALT))
                        prec = ndigs;

                    /*
                     * Figure out whether to use 'f' or 'e' format. The spec
                     * says to use 'f' if the exponent is >= -4 and < requested
                     * precision.
                     */
                    if (-4 <= exp && exp < req_prec)
                    {
                        flags |= FL_FLTFIX;

                        /* Compute how many digits to show after the decimal.
                         *
                         * If exp is negative, then we need to show that
                         * many leading zeros plus the requested precision
                         *
                         * If exp is less than prec, then we need to show a
                         * number of digits past the decimal point,
                         * including (potentially) some trailing zeros
                         *
                         * (these two cases end up computing the same value,
                         * and are both caught by the exp < prec test,
                         * so they share the same branch of the 'if')
                         *
                         * If exp is at least 'prec', then we don't show
                         * any digits past the decimal point.
                         */
                        if (exp < prec)
                            prec = prec - (exp + 1);
                        else
                            prec = 0;
                    } else {
                        /* Compute how many digits to show after the decimal */
                        prec = prec - 1;
                    }
                }

                /* Conversion result length, width := free space length	*/
                if (flags & FL_FLTFIX)
                    n = (exp>0 ? exp+1 : 1);
                else {
                    n = 3;                  /* 1e+ */
#ifdef _NEED_IO_C99_FORMATS
                    if (flags & FL_FLTHEX)
                        n += 2;             /* or 0x1p+ */
#endif
                    n += ndigs_exp;		/* add exponent */
                }
                if (sign)
                    n += 1;
                if (prec)
                    n += prec + 1;
                else if (flags & FL_ALT)
                    n += 1;

                width = width > n ? width - n : 0;

                /* Output before first digit	*/
                if (!(flags & (FL_LPAD | FL_ZFILL))) {
                    while (width) {
                        my_putc (' ', stream);
                        width--;
                    }
                }
                if (sign)
                    my_putc (sign, stream);

#ifdef _NEED_IO_C99_FORMATS
                if ((flags & FL_FLTHEX)) {
                    my_putc('0', stream);
                    my_putc(TOCASE('x'), stream);
                }
#endif

                if (!(flags & FL_LPAD)) {
                    while (width) {
                        my_putc ('0', stream);
                        width--;
                    }
                }

                if (flags & FL_FLTFIX) {		/* 'f' format		*/
                    char out;

                    /* At this point, we should have
                     *
                     *	exp	exponent of leftmost digit in _dtoa.digits
                     *	ndigs	number of buffer digits to print
                     *	prec	number of digits after decimal
                     *
                     * In the loop, 'n' walks over the exponent value
                     */
                    n = exp > 0 ? exp : 0;		/* exponent of left digit */
                    do {

                        /* Insert decimal point at correct place */
                        if (n == -1)
                            my_putc ('.', stream);

                        /* Pull digits from buffer when in-range,
                         * otherwise use 0
                         */
                        if (0 <= exp - n && exp - n < ndigs)
                            out = _dtoa.digits[exp - n];
                        else
                            out = '0';
                        if (--n < -prec) {
                            break;
                        }
                        my_putc (out, stream);
                    } while (1);
                    if (n == exp
                        && (_dtoa.digits[0] > '5'
                            || (_dtoa.digits[0] == '5' && !(_dtoa.flags & DTOA_CARRY))) )
                    {
                        out = '1';
                    }
                    my_putc (out, stream);
                    if ((flags & FL_ALT) && n == -1)
			my_putc('.', stream);
                } else {				/* 'e(E)' format	*/

                    /* mantissa	*/
                    if (_dtoa.digits[0] != '1')
                        _dtoa.flags &= ~DTOA_CARRY;
                    my_putc (_dtoa.digits[0], stream);
                    if (prec > 0) {
                        my_putc ('.', stream);
                        uint8_t pos = 1;
                        for (pos = 1; pos < 1 + prec; pos++)
                            my_putc (pos < ndigs ? _dtoa.digits[pos] : '0', stream);
                    } else if (flags & FL_ALT)
                        my_putc ('.', stream);

                    /* exponent	*/
                    my_putc (TOCASE(c), stream);
                    sign = '+';
                    if (exp < 0 || (exp == 0 && (_dtoa.flags & DTOA_CARRY) != 0)) {
                        exp = -exp;
                        sign = '-';
                    }
                    my_putc (sign, stream);
#ifdef _NEED_IO_LONG_DOUBLE
                    if (ndigs_exp > 4) {
			my_putc(exp / 10000 + '0', stream);
			exp %= 10000;
                    }
#endif
#ifdef _NEED_IO_DOUBLE
                    if (ndigs_exp > 3) {
			my_putc(exp / 1000 + '0', stream);
			exp %= 1000;
                    }
#endif
                    if (ndigs_exp > 2) {
			my_putc(exp / 100 + '0', stream);
			exp %= 100;
                    }
                    if (ndigs_exp > 1) {
                        my_putc(exp / 10 + '0', stream);
                        exp %= 10;
                    }
                    my_putc ('0' + exp, stream);
                }
	    }
#else		/* to: PRINTF_LEVEL >= PRINTF_FLT */
            SKIP_FLOAT_ARG(flags, ap);
	    pnt = "*float*";
	    size = sizeof ("*float*") - 1;
	    goto str_lpad;
#endif
        } else
#endif /* ifndef PRINT_SHRINK */
        {
            int buf_len;
#ifdef WIDE_CHARS
            CHAR *wstr = NULL;
            CHAR c_arg;

            pnt = NULL;
#endif

            if (c == 'c') {
#ifdef WIDE_CHARS
                c_arg = va_arg (ap, int);
                if (!(flags & FL_LONG))
                    c_arg = (char) c_arg;
                wstr = &c_arg;
                size = 1;
                goto str_lpad;
#else
#ifdef _NEED_IO_SHRINK
                my_putc(va_arg(ap, int), stream);
#else
                buf[0] = va_arg (ap, int);
                pnt = buf;
                size = 1;
                goto str_lpad;
#endif
#endif
            } else if (c == 's') {
#ifdef WIDE_CHARS
                if (flags & FL_LONG) {
                    wstr = va_arg(ap, CHAR *);
                    if (wstr) {
                        size = wcsnlen(wstr, (flags & FL_PREC) ? (size_t) prec : SIZE_MAX);
                        goto str_lpad;
                    }
                } else
#endif
                    pnt = va_arg (ap, char *);
                if (!pnt)
                    pnt = "(null)";
#ifdef _NEED_IO_SHRINK
                char c;
                while ( (c = *pnt++) )
                    my_putc(c, stream);
#else
                size = strnlen (pnt, (flags & FL_PREC) ? (size_t) prec : SIZE_MAX);

            str_lpad:
                if (!(flags & FL_LPAD)) {
                    while ((size_t) width > size) {
                        my_putc (' ', stream);
                        width--;
                    }
                }
                width -= size;
                while (size--) {
#ifdef WIDE_CHARS
                    if (wstr)
                        my_putc(*wstr++, stream);
                    else
#endif
                        my_putc (*pnt++, stream);
                }
#endif

            } else {
                if (c == 'd' || c == 'i') {
                    ultoa_signed_t x_s;

                    arg_to_signed(ap, flags, x_s);

                    if (x_s < 0) {
                        x_s = -x_s;
                        flags |= FL_NEGATIVE;
                    }

                    flags &= ~FL_ALT;

#ifndef _NEED_IO_SHRINK
                    if (x_s == 0 && (flags & FL_PREC) && prec == 0)
                        buf_len = 0;
                    else
#endif
                        buf_len = __ultoa_invert (x_s, buf, 10) - buf;
                } else {
                    int base;
                    ultoa_unsigned_t x;

                    if (c == 'u') {
                        flags &= ~FL_ALT;
                        base = 10;
                    } else if (c == 'o') {
                        base = 8;
                        c = '\0';
                    } else if (c == 'p') {
                        base = 16;
                        flags |= FL_ALT;
                        c = 'x';
                        if (sizeof(void *) > sizeof(int))
                            flags |= FL_LONG;
                    } else if (TOLOW(c) == 'x') {
                        base = ('x' - c) | 16;
#ifdef _NEED_IO_PERCENT_B
                    } else if (TOLOW(c) == 'b') {
                        base = 2;
#endif
                    } else {
                        my_putc('%', stream);
                        my_putc(c, stream);
                        continue;
                    }

                    flags &= ~(FL_PLUS | FL_SPACE);

                    arg_to_unsigned(ap, flags, x);

                    /* Zero gets no special alternate treatment */
                    if (x == 0)
                        flags &= ~FL_ALT;

#ifndef _NEED_IO_SHRINK
                    if (x == 0 && (flags & FL_PREC) && prec == 0)
                        buf_len = 0;
                    else
#endif
                        buf_len = __ultoa_invert (x, buf, base) - buf;
                }

#ifndef _NEED_IO_SHRINK
                int len = buf_len;

                /* Specified precision */
                if (flags & FL_PREC) {

                    /* Zfill ignored when precision specified */
                    flags &= ~FL_ZFILL;

                    /* If the number is shorter than the precision, pad
                     * on the left with zeros */
                    if (len < prec) {
                        len = prec;

                        /* Don't add the leading '0' for alternate octal mode */
                        if (c == '\0')
                            flags &= ~FL_ALT;
                    }
                }

                /* Alternate mode for octal/hex */
                if (flags & FL_ALT) {

                    len += 1;
                    if (c != '\0')
                        len += 1;
                } else if (flags & (FL_NEGATIVE | FL_PLUS | FL_SPACE)) {
                    len += 1;
                }

                /* Pad on the left ? */
                if (!(flags & FL_LPAD)) {

                    /* Pad with zeros, using the same loop as the
                     * precision modifier
                     */
                    if (flags & FL_ZFILL) {
                        prec = buf_len;
                        if (len < width) {
                            prec += width - len;
                            len = width;
                        }
                    }
                    while (len < width) {
                        my_putc (' ', stream);
                        len++;
                    }
                }

                /* Width remaining on right after value */
                width -= len;

                /* Output leading characters */
                if (flags & FL_ALT) {
                    my_putc ('0', stream);
                    if (c != '\0')
                        my_putc (c, stream);
                } else if (flags & (FL_NEGATIVE | FL_PLUS | FL_SPACE)) {
                    unsigned char z = ' ';
                    if (flags & FL_PLUS) z = '+';
                    if (flags & FL_NEGATIVE) z = '-';
                    my_putc (z, stream);
                }

                /* Output leading zeros */
                while (prec > buf_len) {
                    my_putc ('0', stream);
                    prec--;
                }
#else
                if (flags & FL_ALT) {
                    my_putc ('0', stream);
                    if (c != '\0')
                        my_putc (c, stream);
                } else if (flags & FL_NEGATIVE)
                    my_putc('-', stream);
#endif

                /* Output value */
                while (buf_len)
                    my_putc (buf[--buf_len], stream);
            }
        }

#ifndef _NEED_IO_SHRINK
	/* Tail is possible.	*/
	while (width-- > 0) {
	    my_putc (' ', stream);
	}
#endif
    } /* for (;;) */

  ret:
#ifdef _NEED_IO_POS_ARGS
    va_end(ap);
#endif
    return stream_len;
#undef my_putc
#undef ap
  fail:
    stream_len = -1;
    goto ret;
}

#if defined(FORMAT_DEFAULT_DOUBLE) && !defined(vfprintf)
#ifdef _HAVE_ALIAS_ATTRIBUTE
__strong_reference(vfprintf, __d_vfprintf);
#else
int __d_vfprintf (FILE * stream, const char *fmt, va_list ap) { return vfprintf(stream, fmt, ap); }
#endif
#endif
