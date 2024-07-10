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

#ifndef PRINTF_NAME
# define PRINTF_VARIANT __IO_VARIANT_DOUBLE
# define PRINTF_NAME __d_vfprintf
#endif

#include "stdio_private.h"
#include "../../libm/common/math_config.h"
#include "../stdlib/local.h"

/*
 * This file can be compiled into more than one flavour by setting
 * PRINTF_VARIANT to:
 *
 *  __IO_VARIANT_MINIMAL: limited integer-only support with option for long long
 *
 *  __IO_VARIANT_INTEGER: integer support except for long long with
 *              options for positional params
 *
 *  __IO_VARIANT_LLONG: full integer support including long long with
 *                options for positional params
 *
 *  __IO_VARIANT_FLOAT: full integer support along with float, but not double
 *
 *  __IO_VARIANT_DOUBLE: full support
 */

# if __IO_DEFAULT != PRINTF_VARIANT || defined(WIDE_CHARS)
#  define vfprintf PRINTF_NAME
# endif

#ifdef WIDE_CHARS
# define CHAR wchar_t
# define UCHAR wchar_t
#else
# define CHAR char
# define UCHAR unsigned char
#endif

/*
 * Compute which features are required. This should match the _HAS
 * values computed in stdio.h
 */

#if PRINTF_VARIANT == __IO_VARIANT_MINIMAL

# define _NEED_IO_SHRINK
# if defined(__IO_MINIMAL_LONG_LONG) && __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# ifdef __IO_C99_FORMATS
#  define _NEED_IO_C99_FORMATS
# endif

#elif PRINTF_VARIANT == __IO_VARIANT_INTEGER

# if defined(__IO_LONG_LONG) && __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# ifdef __IO_POS_ARGS
#  define _NEED_IO_POS_ARGS
# endif
# ifdef __IO_C99_FORMATS
#  define _NEED_IO_C99_FORMATS
# endif
# ifdef __IO_PERCENT_B
#  define _NEED_IO_PERCENT_B
# endif

#elif PRINTF_VARIANT == __IO_VARIANT_LLONG

# if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# ifdef __IO_POS_ARGS
#  define _NEED_IO_POS_ARGS
# endif
# ifdef __IO_C99_FORMATS
#  define _NEED_IO_C99_FORMATS
# endif
# ifdef __IO_PERCENT_B
#  define _NEED_IO_PERCENT_B
# endif

#elif PRINTF_VARIANT == __IO_VARIANT_FLOAT

# if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# define _NEED_IO_POS_ARGS
# define _NEED_IO_C99_FORMATS
# ifdef __IO_PERCENT_B
#  define _NEED_IO_PERCENT_B
# endif
# define _NEED_IO_FLOAT

#elif PRINTF_VARIANT == __IO_VARIANT_DOUBLE

# if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#  define _NEED_IO_LONG_LONG
# endif
# if defined(_HAS_IO_WCHAR) || defined(WIDE_CHARS)
#  define _NEED_IO_WCHAR
# endif
# define _NEED_IO_POS_ARGS
# define _NEED_IO_C99_FORMATS
# ifdef __IO_PERCENT_B
#  define _NEED_IO_PERCENT_B
# endif
# define _NEED_IO_DOUBLE
# if defined(__IO_LONG_DOUBLE) && __SIZEOF_LONG_DOUBLE__ > __SIZEOF_DOUBLE__
#  define _NEED_IO_LONG_DOUBLE
# endif

#else

# error invalid PRINTF_VARIANT

#endif

/* Figure out which multi-byte char support we need */
#if defined(_NEED_IO_WCHAR) && defined(__MB_CAPABLE)
# ifdef WIDE_CHARS
/* need to convert multi-byte chars to wide chars */
#  define _NEED_IO_MBTOWIDE
# else
#  define _NEED_IO_WIDETOMB
# endif
#endif

#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
#include "dtoa.h"
#endif

#if defined(_NEED_IO_FLOAT_LONG_DOUBLE) &&  __SIZEOF_LONG_DOUBLE__ > __SIZEOF_DOUBLE__
#define SKIP_FLOAT_ARG(flags, ap) do {                                  \
        if ((flags & (FL_LONG | FL_REPD_TYPE)) == (FL_LONG | FL_REPD_TYPE)) \
            (void) va_arg(ap, long double);                             \
        else                                                            \
            (void) va_arg(ap, double);                                  \
    } while(0)
#define FLOAT double
#elif defined(_NEED_IO_FLOAT)
#define SKIP_FLOAT_ARG(flags, ap) (void) va_arg(ap, uint32_t)
#define FLOAT float
#else
#define SKIP_FLOAT_ARG(flags, ap) (void) va_arg(ap, double)
#define FLOAT double
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

#ifdef _NEED_IO_C99_FORMATS

#define CHECK_INT_SIZE(c, flags, letter, type)          \
    if (c == letter) {                                  \
        if (sizeof(type) == sizeof(int))                \
            ;                                           \
        else if (sizeof(type) == sizeof(long))          \
            flags |= FL_LONG;                           \
        else if (sizeof(type) == sizeof(long long))     \
            flags |= FL_LONG|FL_REPD_TYPE;              \
        else if (sizeof(type) == sizeof(short))         \
            flags |= FL_SHORT;                          \
        continue;                                       \
    }

#define CHECK_C99_INT_SIZES(c, flags)           \
    CHECK_INT_SIZE(c, flags, 'j', intmax_t);    \
    CHECK_INT_SIZE(c, flags, 'z', size_t);      \
    CHECK_INT_SIZE(c, flags, 't', ptrdiff_t);

#else
#define CHECK_C99_INT_SIZES(c, flags)
#endif

#define CHECK_INT_SIZES(c, flags) {             \
        if (c == 'l') {                         \
            if (flags & FL_LONG)                \
                flags |= FL_REPD_TYPE;          \
            flags |= FL_LONG;                   \
            continue;                           \
        }                                       \
                                                \
        if (c == 'h') {                         \
            if (flags & FL_SHORT)               \
                flags |= FL_REPD_TYPE;          \
            flags |= FL_SHORT;                  \
            continue;                           \
        }                                       \
                                                \
        /* alias for 'll' */                    \
        if (c == 'L') {                         \
            flags |= FL_REPD_TYPE;              \
            flags |= FL_LONG;                   \
            continue;                           \
        }                                       \
        CHECK_C99_INT_SIZES(c, flags);          \
    }

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

            CHECK_INT_SIZES(c, flags);

	    break;
	} while ( (c = *fmt++) != 0);
        if (argno == 0)
            break;
        if (argno == current_argno) {
            if ((TOLOWER(c) >= 'e' && TOLOWER(c) <= 'g')
#ifdef _NEED_IO_C99_FORMATS
                || TOLOWER(c) == 'a'
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

#ifdef _NEED_IO_WIDETOMB
/*
 * Compute the number of bytes to encode a wide string
 * in the current locale
 */
static size_t
_mbslen(const wchar_t *s, size_t maxlen)
{
    mbstate_t ps = {0};
    wchar_t c;
    char tmp[MB_LEN_MAX];
    size_t len = 0;
    while (len < maxlen && (c = *s++) != L'\0') {
        int clen;
        clen = __WCTOMB (tmp, c, &ps);
        if (clen == -1)
            return (size_t) clen;
        len += clen;
    }
    return len;
}

#endif

#ifdef _NEED_IO_MBTOWIDE
/*
 * Compute the number of wide chars to encode a multi-byte string
 * in the current locale
 */
static size_t
_wcslen(const char *s, size_t maxlen)
{
    mbstate_t ps = {0};
    wchar_t c;
    size_t len = 0;
    while (len < maxlen && *s != '\0') {
        size_t clen = mbrtowc(&c, s, MB_LEN_MAX, &ps);
        if (c == L'\0')
            break;
        if (clen == (size_t) -1)
            return clen;
        s += clen;
        len++;
    }
    return len;
}
#endif

#ifdef VFPRINTF_S
int
vfprintf_s(FILE *__restrict stream, const char *__restrict fmt, va_list ap_orig)
#else
int vfprintf (FILE * stream, const CHAR *fmt, va_list ap_orig)
#endif
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
#ifdef _NEED_IO_WCHAR
        wchar_t __wbuf[PRINTF_BUF_SIZE/2]; /* for wide char output */
#ifdef _NEED_IO_WIDETOMB
        char __mb[MB_LEN_MAX];
#endif
#endif
#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
	struct dtoa __dtoa;
#endif
    } u;
    const char * pnt;
#ifndef _NEED_IO_SHRINK
    size_t size;
#endif

#define buf	(u.__buf)
#define wbuf    (u.__wbuf)
#define dtoa	(u.__dtoa)

    int stream_len = 0;

#ifdef VFPRINTF_S
    const char *msg = "";

    if (stream == NULL) {
        msg = "output stream is null";
        goto handle_error;
    } else if (fmt == NULL) {
        msg = "null format string";
        goto handle_error;
    }
#endif

#ifndef my_putc
#ifdef WIDE_CHARS
#define my_putc(c, stream) do { ++stream_len; if (putwc(c, stream) == WEOF) goto fail; } while(0)
#else
    int (*put)(char, FILE *) = stream->put;
#define my_putc(c, stream) do { ++stream_len; if (put(c, stream) < 0) goto fail; } while(0)
#endif
#endif

    __flockfile(stream);

    if ((stream->flags & __SWR) == 0)
	__funlock_return(stream, EOF);

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
#ifdef _NEED_IO_WCHAR
        wchar_t *wstr = NULL;
#endif

	do {
	    if (flags < FL_WIDTH) {
		switch (c) {
		  case '0':
		    flags |= FL_ZFILL;
		    continue;
		  case '+':
		    flags |= FL_PLUS;
		    __fallthrough;
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

            CHECK_INT_SIZES(c, flags);

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

#define TOCASE(c)       ((c) - case_convert)

#ifndef _NEED_IO_SHRINK
	if ((TOLOWER(c) >= 'e' && TOLOWER(c) <= 'g')
#ifdef _NEED_IO_C99_FORMATS
            || TOLOWER(c) == 'a'
#endif
            )
        {
#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
            uint8_t sign;		/* sign character (or 0)	*/
            uint8_t ndigs;		/* number of digits to convert */
            unsigned char case_convert; /* subtract to correct case */
	    int exp;			/* exponent of most significant decimal digit */
	    int n;                      /* total width */
	    uint8_t ndigs_exp;		/* number of digis in exponent */

            /* deal with upper vs lower case */
            case_convert = TOLOWER(c) - c;
            c = TOLOWER(c);

#ifdef _NEED_IO_LONG_DOUBLE
            if ((flags & (FL_LONG | FL_REPD_TYPE)) == (FL_LONG | FL_REPD_TYPE))
            {
                long double fval;
                fval = va_arg(ap, long double);

                ndigs = 0;

#ifdef _NEED_IO_C99_FORMATS
                if (c == 'a') {

                    c = 'p';
                    flags |= FL_FLTEXP | FL_FLTHEX;

                    if (!(flags & FL_PREC))
                        prec = -1;
                    prec = __lfloat_x_engine(fval, &dtoa, prec, case_convert);
                    ndigs = prec + 1;
                    exp = dtoa.exp;
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
                        ndigs = LONG_FLOAT_MAX_DIG;
                        ndecimal = prec;
                        flags |= FL_FLTFIX;
                        fmode = true;
                    } else {
                        c += 'e' - 'g';
                        ndigs = prec;
                        if (ndigs < 1) ndigs = 1;
                    }

                    if (ndigs > LONG_FLOAT_MAX_DIG)
                        ndigs = LONG_FLOAT_MAX_DIG;

                    ndigs = __lfloat_d_engine(fval, &dtoa, ndigs, fmode, ndecimal);

                    exp = dtoa.exp;
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

                    c = 'p';
                    flags |= FL_FLTEXP | FL_FLTHEX;

                    if (!(flags & FL_PREC))
                        prec = -1;

                    ndigs = 1 + __float_x_engine(fval, &dtoa, prec, case_convert);
                    if (prec <= ndigs)
                        prec = ndigs - 1;
                    exp = dtoa.exp;
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
                        ndigs = FLOAT_MAX_DIG;
                        ndecimal = prec;
                        flags |= FL_FLTFIX;
                        fmode = true;
                    } else {
                        c += 'e' - 'g';
                        ndigs = prec;
                        if (ndigs < 1) ndigs = 1;
                    }

                    if (ndigs > FLOAT_MAX_DIG)
                        ndigs = FLOAT_MAX_DIG;

                    ndigs = __float_d_engine (fval, &dtoa, ndigs, fmode, ndecimal);
                    exp = dtoa.exp;
                    ndigs_exp = 2;
                }
            }
	    if (exp < -9 || 9 < exp)
		    ndigs_exp = 2;
	    if (exp < -99 || 99 < exp)
		    ndigs_exp = 3;
#ifdef _NEED_IO_FLOAT64
	    if (exp < -999 || 999 < exp)
		    ndigs_exp = 4;
#ifdef _NEED_IO_FLOAT_LARGE
	    if (exp < -9999 || 9999 < exp)
		    ndigs_exp = 5;
#endif
#endif

	    sign = 0;
	    if (dtoa.flags & DTOA_MINUS)
		sign = '-';
	    else if (flags & FL_PLUS)
		sign = '+';
	    else if (flags & FL_SPACE)
		sign = ' ';

	    if (dtoa.flags & (DTOA_NAN | DTOA_INF))
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
		if (dtoa.flags & DTOA_NAN)
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
                    while (ndigs > 0 && dtoa.digits[ndigs-1] == '0')
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
                     *	exp	exponent of leftmost digit in dtoa.digits
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
                            out = dtoa.digits[exp - n];
                        else
                            out = '0';
                        if (--n < -prec) {
                            break;
                        }
                        my_putc (out, stream);
                    } while (1);
                    my_putc (out, stream);
                    if ((flags & FL_ALT) && n == -1)
			my_putc('.', stream);
                } else {				/* 'e(E)' format	*/

                    /* mantissa	*/
                    my_putc (dtoa.digits[0], stream);
                    if (prec > 0) {
                        my_putc ('.', stream);
                        int pos = 1;
                        for (pos = 1; pos < 1 + prec; pos++)
                            my_putc (pos < ndigs ? dtoa.digits[pos] : '0', stream);
                    } else if (flags & FL_ALT)
                        my_putc ('.', stream);

                    /* exponent	*/
                    my_putc (TOCASE(c), stream);
                    sign = '+';
                    if (exp < 0) {
                        exp = -exp;
                        sign = '-';
                    }
                    my_putc (sign, stream);
#ifdef _NEED_IO_FLOAT_LARGE
                    if (ndigs_exp > 4) {
			my_putc(exp / 10000 + '0', stream);
			exp %= 10000;
                    }
#endif
#ifdef _NEED_IO_FLOAT64
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
#else		/* to: IO_VARIANT_IS_FLOAT(PRINTF_VARIANT) */
            SKIP_FLOAT_ARG(flags, ap);
	    pnt = "*float*";
	    size = sizeof ("*float*") - 1;
	    goto str_lpad;
#endif
        } else
#endif /* ifndef PRINT_SHRINK */
        {
            int buf_len;
#ifdef _NEED_IO_WCHAR
            pnt = NULL;
#endif

            if (c == 'c') {
#ifdef _NEED_IO_SHRINK
                my_putc(va_arg(ap, int), stream);
#else
#ifdef _NEED_IO_WCHAR
                if (flags & FL_LONG) {
                    wbuf[0] = va_arg (ap, wint_t);
                    wbuf[1] = L'\0';
                    wstr = wbuf;
                    goto wstr_lpad;
                }
#endif
                buf[0] = va_arg (ap, int);
                pnt = buf;
                size = 1;
                goto str_lpad;
#endif
            } else if (c == 's') {
#ifdef _NEED_IO_WCHAR
                if (flags & FL_LONG) {
                    wstr = va_arg(ap, wchar_t *);
                    if (wstr) {
                    wstr_lpad:
                        size = (flags & FL_PREC) ? (size_t) prec : SIZE_MAX;
#ifdef _NEED_IO_WIDETOMB
                        size = _mbslen(wstr, size);
                        if (size == (size_t) -1)
                            goto ret;
#else
                        size = wcsnlen(wstr, size);
#endif
                        goto str_lpad;
                    }
                } else
#endif
                    pnt = va_arg (ap, char *);
                if (!pnt) {
#ifdef VFPRINTF_S
                    msg = "arg corresponding to '%s' is null";
                    goto handle_error;
#endif
                    pnt = "(null)";
                }
#ifdef _NEED_IO_SHRINK
                char c;
                while ( (c = *pnt++) )
                    my_putc(c, stream);
#else
                size = (flags & FL_PREC) ? (size_t) prec : SIZE_MAX;
#ifdef _NEED_IO_MBTOWIDE
                size = _wcslen (pnt, size);
#else
                size = strnlen (pnt, size);
#endif
            str_lpad:
                if (!(flags & FL_LPAD)) {
                    while ((size_t) width > size) {
                        my_putc (' ', stream);
                        width--;
                    }
                }
                width -= size;
#ifdef _NEED_IO_WCHAR
                if (wstr) {
#ifdef _NEED_IO_WIDETOMB
                    mbstate_t   ps = {0};
                    while(size) {
                        wchar_t c = *wstr++;
                        char *m = u.__mb;
                        int mb_len = __WCTOMB(m, c, &ps);
                        while (size && mb_len) {
                            my_putc(*m++, stream);
                            size--;
                            mb_len--;
                        }
                    }
#else
                    while(size--)
                        my_putc(*wstr++, stream);
#endif
                } else
#endif
                {
#ifdef _NEED_IO_MBTOWIDE
                    mbstate_t   ps = {0};
                    while (size--) {
                        wchar_t c;
                        size_t mb_len = mbrtowc(&c, pnt, MB_LEN_MAX, &ps);
                        my_putc(c, stream);
                        pnt += mb_len;
                    }
#else
                    while (size--)
                        my_putc (*pnt++, stream);
#endif
                }
#endif
#if defined(__IO_PERCENT_N) || defined(VFPRINTF_S)
            } else if (c == 'n') {
#ifdef VFPRINTF_S
                msg = "format string contains percent-n";
                goto handle_error;
#else
                if (flags & FL_LONG) {
                    if (flags & FL_REPD_TYPE)
                        *va_arg(ap, long long *) = stream_len;
                    else
                        *va_arg(ap, long *) = stream_len;
                } else if (flags & FL_SHORT) {
                    if (flags & FL_REPD_TYPE)
                        *va_arg(ap, char *) = stream_len;
                    else
                        *va_arg(ap, short *) = stream_len;
                } else {
                    *va_arg(ap, int *) = stream_len;
                }
#endif
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
                    } else if (TOLOWER(c) == 'x') {
                        base = ('x' - c) | 16;
#ifdef _NEED_IO_PERCENT_B
                    } else if (TOLOWER(c) == 'b') {
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
    __funlock_return(stream, stream_len);
#undef my_putc
#undef ap
  fail:
    stream->flags |= __SERR;
    stream_len = -1;
    goto ret;
#ifdef VFPRINTF_S
  handle_error:
    if (__cur_handler != NULL) {
        __cur_handler(msg, NULL, -1);
    }
    if (stream)
        stream->flags |= __SERR;
    stream_len = -1;
    goto ret;
#endif
}

#if !defined(VFPRINTF_S) && !defined(WIDE_CHARS)
# if PRINTF_VARIANT == __IO_DEFAULT
#  undef vfprintf
#  ifdef __strong_reference
__strong_reference(vfprintf, PRINTF_NAME);
#  else
int PRINTF_NAME (FILE * stream, const char *fmt, va_list ap) { return vfprintf(stream, fmt, ap); }
#  endif
# endif
#endif
