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
# if __SIZEOF_WCHAR_T__ == 2
#  define UCHAR          uint16_t
# elif __SIZEOF_WCHAR_T__ == 4
#  define UCHAR          uint32_t
# endif
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
#define FLOAT_UINT uint32_t
#else
#define SKIP_FLOAT_ARG(flags, ap) (void) va_arg(ap, double)
#if __SIZEOF_DOUBLE__ == 8
#define FLOAT_UINT uint64_t
#elif __SIZEOF_DOUBLE__ == 4
#define FLOAT_UINT uint32_t
#endif
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
#ifdef _NEED_IO_POS_ARGS
    int argno;
    my_va_list my_ap;
    const CHAR *fmt_orig = fmt;
#define ap my_ap.ap
#else
#define ap ap_orig
#endif
    union {
	char buf[PRINTF_BUF_SIZE];	/* size for -1 in smallest base, without '\0'	*/
#ifdef _NEED_IO_WCHAR
        wchar_t wbuf[PRINTF_BUF_SIZE/2]; /* for wide char output */
#endif
#ifdef _NEED_IO_WIDETOMB
        char mb[MB_LEN_MAX];
#endif
#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
	struct dtoa dtoa;
#endif
    } u;
    const char * pnt;
#ifndef _NEED_IO_SHRINK
    size_t size;
#endif

    int stream_len = 0;

#ifdef VFPRINTF_S
    const char *msg;

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
#ifndef _NEED_IO_SHRINK
	int width = 0;
	int prec = 0;
#endif
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
		    } else {
                        width = 10*width + c;
                        flags |= FL_WIDTH;
                    }
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
#ifdef _NEED_IO_SHRINK
                        (void) va_arg(ap, int);
#else
			prec = va_arg(ap, int);
#endif
		    } else {
#ifdef _NEED_IO_SHRINK
			(void) va_arg(ap, int);
#else
			width = va_arg(ap, int);
#endif
			flags |= FL_WIDTH;
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

        /* Handle negative width when provided via a '*' */
        if (width < 0) {
            width = -width;
            flags |= FL_LPAD;
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
#include "vfprintf_float.c"
#else
            SKIP_FLOAT_ARG(flags, ap);
            pnt = "*float*";
            size = sizeof ("*float*") - 1;
            goto str_lpad;
#endif
        }
        else
#endif /* ifndef PRINT_SHRINK */
        {
            int buf_len;
#ifdef _NEED_IO_WCHAR
            wchar_t *wstr = NULL;
#endif
#ifdef _NEED_IO_WCHAR
            pnt = NULL;
#endif

            if (c == 'c') {
#include "vfprintf_char.c"
            }

            else if (c == 's') {
#include "vfprintf_str.c"
            }

#if defined(__IO_PERCENT_N) || defined(VFPRINTF_S)
            else if (c == 'n') {
#include "vfprintf_n.c"
            }
#endif

            else {
#include "vfprintf_int.c"
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
