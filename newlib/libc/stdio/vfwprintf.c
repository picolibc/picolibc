/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
FUNCTION
<<vfwprintf>>, <<vwprintf>>, <<vswprintf>>---wide character format argument list

INDEX
	vfwprintf
INDEX
	vwprintf
INDEX
	vswprintf

SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	#include <wchar.h>
	int vwprintf(const wchar_t *__restrict <[fmt]>, va_list <[list]>);
	int vfwprintf(FILE *__restrict <[fp]>,
		const wchar_t *__restrict <[fmt]>, va_list <[list]>);
	int vswprintf(wchar_t * __restrict <[str]>, size_t <[size]>,
		const wchar_t *__ restrict <[fmt]>, va_list <[list]>);

	int vwprintf( const wchar_t *<[fmt]>,
		va_list <[list]>);
	int vfwprintf( FILE *<[fp]>,
		const wchar_t *<[fmt]>, va_list <[list]>);
	int vswprintf( wchar_t *<[str]>,
		size_t <[size]>, const wchar_t *<[fmt]>, va_list <[list]>);

DESCRIPTION
<<vwprintf>>, <<vfwprintf>> and <<vswprintf>> are (respectively) variants
of <<wprintf>>, <<fwprintf>> and <<swprintf>>.  They differ only in allowing
their caller to pass the variable argument list as a <<va_list>> object
(initialized by <<va_start>>) rather than directly accepting a variable
number of arguments.  The caller is responsible for calling <<va_end>>.

RETURNS
The return values are consistent with the corresponding functions.

PORTABILITY
POSIX-1.2008 with extensions; C99 (compliant except for POSIX extensions).

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.

SEEALSO
<<wprintf>>, <<fwprintf>> and <<swprintf>>.
*/

/*
 * Actual wprintf innards.
 *
 * This code is large and complicated...
 */
#define _DEFAULT_SOURCE
#include <newlib.h>

#ifdef INTEGER_ONLY
# ifdef STRING_ONLY
#   define VFWPRINTF svfiwprintf
# else
#   define VFWPRINTF vfiwprintf
# endif
#else
# ifdef STRING_ONLY
#   define VFWPRINTF svfwprintf
# else
#   define VFWPRINTF vfwprintf
# endif
# ifndef NO_FLOATING_POINT
#  define FLOATING_POINT
# endif
#endif

#define _NO_POS_ARGS
#ifdef _WANT_IO_POS_ARGS
# undef _NO_POS_ARGS
#endif

#define _DEFAULT_SOURCE
#include <_ansi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <wchar.h>
#include <sys/lock.h>
#include <stdarg.h>
#include "local.h"
#include "fvwrite.h"
#include "vfieeefp.h"
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
#include "../locale/setlocale.h"
#endif

int VFWPRINTF (FILE *, const wchar_t *, va_list);

/* Currently a test is made to see if long double processing is warranted.
   This could be changed in the future should the __ldtoa code be
   preferred over __dtoa.  */
#define _NO_LONGDBL
#if defined _WANT_IO_LONG_DOUBLE && (LDBL_MANT_DIG > DBL_MANT_DIG)
#undef _NO_LONGDBL
#endif

#define _NO_LONGLONG
#if defined _WANT_IO_LONG_LONG \
	&& (defined __GNUC__ || __STDC_VERSION__ >= 199901L)
# undef _NO_LONGLONG
#endif

/* Defined in vfprintf.c. */
#ifdef _FVWRITE_IN_STREAMIO
# ifdef STRING_ONLY
#  define __SPRINT __sswprint
# else
#  define __SPRINT __swprint
# endif
int __SPRINT (FILE *, register struct __suio *);
#else
# ifdef STRING_ONLY
#  define __SPRINT __ssputws
# else
#  define __SPRINT __sfputws
# endif
int __SPRINT (FILE *, const wchar_t *, size_t);
#endif
#ifndef STRING_ONLY
#ifdef _UNBUF_STREAM_OPT
/*
 * Helper function for `fprintf to unbuffered unix file': creates a
 * temporary buffer.  We only work on write-only files; this avoids
 * worries about ungetc buffers and so forth.
 */
static int
__sbwprintf (
       register FILE *fp,
       const wchar_t *fmt,
       va_list ap)
{
	int ret;
	FILE fake;
	unsigned char buf[BUFSIZ];

	/* copy the important variables */
	fake._flags = fp->_flags & ~__SNBF;
	fake._flags2 = fp->_flags2;
	fake._file = fp->_file;
	fake._cookie = fp->_cookie;
	fake._write = fp->_write;

	/* set up the buffer */
	fake._bf._base = fake._p = buf;
	fake._bf._size = fake._w = sizeof (buf);
	fake._lbfsize = 0;	/* not actually used, but Just In Case */
#ifndef __SINGLE_THREAD__
	__lock_init_recursive (fake._lock);
#endif

	/* do the work, then copy any error status */
	ret = VFWPRINTF (&fake, fmt, ap);
	if (ret >= 0 && fflush ( &fake))
		ret = EOF;
	if (fake._flags & __SERR)
		fp->_flags |= __SERR;

#ifndef __SINGLE_THREAD__
	__lock_close_recursive (fake._lock);
#endif
	return (ret);
}
#endif /* _UNBUF_STREAM_OPT */
#endif /* !STRING_ONLY */


#if defined (FLOATING_POINT) || defined (_WANT_IO_C99_FORMATS)
# include <locale.h>
#endif
#ifdef FLOATING_POINT
# include <math.h>

/* For %La, an exponent of 15 bits occupies the exponent character, a
   sign, and up to 5 digits.  */
# define MAXEXPLEN		7
# define DEFPREC		6

# ifdef _NO_LONGDBL

#  define _PRINTF_FLOAT_TYPE double
#  define _DTOA __dtoa
#  define FREXP frexp

# else /* !_NO_LONGDBL */

extern int _ldcheck (_LONG_DOUBLE *);

#  define _PRINTF_FLOAT_TYPE _LONG_DOUBLE
#  define _DTOA __ldtoa
#  define FREXP frexpl
# endif /* !_NO_LONGDBL */

static wchar_t *wcvt(_PRINTF_FLOAT_TYPE, int, int, wchar_t *,
		    int *, int, int *, wchar_t *, int);

static int wexponent(wchar_t *, int, int);

#endif /* FLOATING_POINT */

/* BUF must be big enough for the maximum %#llo (assuming long long is
   at most 64 bits, this would be 23 characters), the maximum
   multibyte character %C, and the maximum default precision of %La
   (assuming long double is at most 128 bits with 113 bits of
   mantissa, this would be 29 characters).  %e, %f, and %g use
   reentrant storage shared with mprec.  All other formats that use
   buf get by with fewer characters.  Making BUF slightly bigger
   reduces the need for malloc in %.*a and %ls/%S, when large precision or
   long strings are processed.
   The bigger size of 100 bytes is used on systems which allow number
   strings using the locale's grouping character.  Since that's a multibyte
   value, we should use a conservative value.
   */
#ifdef _WANT_IO_C99_FORMATS
#define BUF             100
#else
#define	BUF		40
#endif
#if defined _MB_CAPABLE && MB_LEN_MAX > BUF
# undef BUF
# define BUF MB_LEN_MAX
#endif

#ifndef _NO_LONGLONG
# define quad_t long long
# define u_quad_t unsigned long long
#else
# define quad_t long
# define u_quad_t unsigned long
#endif

typedef quad_t * quad_ptr_t;
typedef void *void_ptr_t;
typedef char *   char_ptr_t;
typedef wchar_t* wchar_ptr_t;
typedef long *   long_ptr_t;
typedef int  *   int_ptr_t;
typedef short *  short_ptr_t;

#ifndef _NO_POS_ARGS
# ifdef NL_ARGMAX
#  define MAX_POS_ARGS NL_ARGMAX
# else
#  define MAX_POS_ARGS 32
# endif

union arg_val
{
  int val_int;
  u_int val_u_int;
  long val_long;
  u_long val_u_long;
  float val_float;
  double val_double;
  _LONG_DOUBLE val__LONG_DOUBLE;
  int_ptr_t val_int_ptr_t;
  short_ptr_t val_short_ptr_t;
  long_ptr_t val_long_ptr_t;
  char_ptr_t val_char_ptr_t;
  wchar_ptr_t val_wchar_ptr_t;
  quad_ptr_t val_quad_ptr_t;
  void_ptr_t val_void_ptr_t;
  quad_t val_quad_t;
  u_quad_t val_u_quad_t;
  wint_t val_wint_t;
};

typedef struct {
    va_list ap;
} my_va_list;

static union arg_val *
get_arg (int n, wchar_t *fmt,
                 my_va_list *my_ap, int *numargs, union arg_val *args,
                 int *arg_type, wchar_t **last_fmt);
#endif /* !_NO_POS_ARGS */

/*
 * Macros for converting digits to letters and vice versa
 */
#define	to_digit(c)	((c) - L'0')
#define is_digit(c)	((unsigned)to_digit (c) <= 9)
#define	to_char(n)	((n) + L'0')

/*
 * Flags used during conversion.
 */
#define	ALT		0x001		/* alternate form */
#define	HEXPREFIX	0x002		/* add 0x or 0X prefix */
#define	LADJUST		0x004		/* left adjustment */
#define	LONGDBL		0x008		/* long double */
#define	LONGINT		0x010		/* long integer */
#ifndef _NO_LONGLONG
# define QUADINT	0x020		/* quad integer */
#else /* ifdef _NO_LONGLONG, make QUADINT equivalent to LONGINT, so
	 that %lld behaves the same as %ld, not as %d, as expected if:
	 sizeof (long long) = sizeof long > sizeof int  */
# define QUADINT	LONGINT
#endif
#define	SHORTINT	0x040		/* short integer */
#define	ZEROPAD		0x080		/* zero (as opposed to blank) pad */
#define FPT		0x100		/* Floating point number */
#ifdef _WANT_IO_C99_FORMATS
# define CHARINT	0x200		/* char as integer */
#else /* define as 0, to make SARG and UARG occupy fewer instructions  */
# define CHARINT	0
#endif
#ifdef _WANT_IO_C99_FORMATS
# define GROUPING	0x400		/* use grouping ("'" flag) */
#endif

int
VFWPRINTF (
       FILE * fp,
       const wchar_t *fmt0,
       va_list ap)
{
	register wchar_t *fmt;	/* format string */
	register wchar_t ch;	/* character from fmt */
	register int n, m;	/* handy integers (short term usage) */
	register wchar_t *cp;	/* handy char pointer (short term usage) */
	register int flags;	/* flags as above */
#ifndef _NO_POS_ARGS
	wchar_t *fmt_anchor;    /* current format spec being processed */
	int N;                  /* arg number */
	int arg_index;          /* index into args processed directly */
	int numargs;            /* number of varargs read */
	wchar_t *saved_fmt;     /* saved fmt pointer */
        my_va_list my_ap;
        va_copy(my_ap.ap, ap);
	union arg_val args[MAX_POS_ARGS];
	int arg_type[MAX_POS_ARGS];
	int is_pos_arg;         /* is current format positional? */
	int old_is_pos_arg;     /* is current format positional? */
#endif
	int ret;		/* return value accumulator */
	int width;		/* width from format (%8d), or 0 */
	int prec;		/* precision from format (%.3d), or -1 */
	wchar_t sign;		/* sign prefix (' ', '+', '-', or \0) */
#ifdef _WANT_IO_C99_FORMATS
				/* locale specific numeric grouping */
	wchar_t thousands_sep = L'\0';
	const char *grouping = NULL;
#endif
#if defined (_MB_CAPABLE) && !defined (__HAVE_LOCALE_INFO_EXTENDED__) \
    && (defined (FLOATING_POINT) || defined (_WANT_IO_C99_FORMATS))
	mbstate_t state;        /* mbtowc calls from library must not change state */
#endif
#ifdef FLOATING_POINT
	wchar_t decimal_point;
	wchar_t softsign;		/* temporary negative sign for floats */
	union { int i; _PRINTF_FLOAT_TYPE fp; } _double_ = {0};
# define _fpvalue (_double_.fp)
	int expt;		/* integer value of exponent */
	int expsize = 0;	/* character count for expstr */
	wchar_t expstr[MAXEXPLEN];	/* buffer for exponent string */
	int lead;		/* sig figs before decimal or group sep */
#endif /* FLOATING_POINT */
#if defined (FLOATING_POINT) || defined (_WANT_IO_C99_FORMATS)
	int ndig = 0;		/* actual number of digits returned by cvt */
#endif
#if defined (FLOATING_POINT) && defined (_WANT_IO_C99_FORMATS)
	int nseps;		/* number of group separators with ' */
	int nrepeats;		/* number of repeats of the last group */
#endif
	u_quad_t _uquad;	/* integer arguments %[diouxX] */
	enum { OCT, DEC, HEX } base;/* base for [diouxX] conversion */
	int dprec;		/* a copy of prec if [diouxX], 0 otherwise */
	int realsz;		/* field size expanded by dprec */
	int size = 0;		/* size of converted field or string */
	wchar_t *xdigs = NULL;	/* digits for [xX] conversion */
#ifdef _FVWRITE_IN_STREAMIO
#define NIOV 8
	struct __suio uio;	/* output information: summary */
	struct __siov iov[NIOV];/* ... and individual io vectors */
	register struct __siov *iovp;/* for PRINT macro */
#endif
	wchar_t buf[BUF];	/* space for %c, %ls/%S, %[diouxX], %[aA] */
	wchar_t ox[2];		/* space for 0x hex-prefix */
	wchar_t *malloc_buf = NULL;/* handy pointer for malloced buffers */

	/*
	 * Choose PADSIZE to trade efficiency vs. size.  If larger printf
	 * fields occur frequently, increase PADSIZE and make the initialisers
	 * below longer.
	 */
#define	PADSIZE	16		/* pad chunk size */
	static const wchar_t blanks[PADSIZE] =
	 {L' ',L' ',L' ',L' ',L' ',L' ',L' ',L' ',
	  L' ',L' ',L' ',L' ',L' ',L' ',L' ',L' '};
	static const wchar_t zeroes[PADSIZE] =
	 {L'0',L'0',L'0',L'0',L'0',L'0',L'0',L'0',
	  L'0',L'0',L'0',L'0',L'0',L'0',L'0',L'0'};

#ifdef FLOATING_POINT
#ifdef _MB_CAPABLE
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
	decimal_point = *__get_current_numeric_locale ()->wdecimal_point;
#else
	{
	  size_t nconv;

	  memset (&state, '\0', sizeof (state));
	  nconv = mbrtowc (&decimal_point,
			      localeconv ()->decimal_point,
			      MB_CUR_MAX, &state);
	  if (nconv == (size_t) -1 || nconv == (size_t) -2)
	    decimal_point = L'.';
	}
#endif
#else
	decimal_point = (wchar_t) *localeconv ()->decimal_point;
#endif
#endif
	/*
	 * BEWARE, these `goto error' on error, and PAD uses `n'.
	 */
#ifdef _FVWRITE_IN_STREAMIO
#define	PRINT(ptr, len) { \
	iovp->iov_base = (char *) (ptr); \
	iovp->iov_len = (len); \
	uio.uio_resid += iovp->iov_len; \
	iovp++; \
	if (++uio.uio_iovcnt >= NIOV) { \
		if (__SPRINT(fp, &uio)) \
			goto error; \
		iovp = iov; \
	} \
}
#define	PAD(howmany, with) { \
	if ((n = (howmany)) > 0) { \
		while (n > PADSIZE) { \
			PRINT (with, PADSIZE); \
			n -= PADSIZE; \
		} \
		PRINT (with, n); \
	} \
}
#define PRINTANDPAD(p, ep, len, with) { \
	int n = (ep) - (p); \
	if (n > (len)) \
		n = (len); \
	if (n > 0) \
		PRINT((p), n); \
	PAD((len) - (n > 0 ? n : 0), (with)); \
}
#define	FLUSH() { \
	if (uio.uio_resid && __SPRINT(fp, &uio)) \
		goto error; \
	uio.uio_iovcnt = 0; \
	iovp = iov; \
}
#else
#define PRINT(ptr, len) {		\
	if (__SPRINT (fp, (ptr), (len)) == EOF) \
		goto error;		\
}
#define	PAD(howmany, with) {		\
	if ((n = (howmany)) > 0) {	\
		while (n > PADSIZE) {	\
			PRINT (with, PADSIZE);	\
			n -= PADSIZE;	\
		}			\
		PRINT (with, n);	\
	}				\
}
#define PRINTANDPAD(p, ep, len, with) {	\
	int n = (ep) - (p);		\
	if (n > (len))			\
		n = (len);		\
	if (n > 0)			\
		PRINT((p), n);		\
	PAD((len) - (n > 0 ? n : 0), (with)); \
}
#define	FLUSH()
#endif

	/* Macros to support positional arguments */
#ifndef _NO_POS_ARGS
# define GET_ARG(n, ap, type)						\
	(is_pos_arg							\
	 ? (n < numargs							\
	    ? args[n].val_##type					\
	    : get_arg (n, fmt_anchor, &my_ap, &numargs, args,           \
		       arg_type, &saved_fmt)->val_##type)		\
	 : (arg_index++ < numargs					\
	    ? args[n].val_##type					\
	    : (numargs < MAX_POS_ARGS					\
	       ? args[numargs++].val_##type = va_arg (my_ap.ap, type)  \
	       : va_arg (my_ap.ap, type))))
#else
# define GET_ARG(n, ap, type) (va_arg (ap, type))
#endif

	/*
	 * To extend shorts properly, we need both signed and unsigned
	 * argument extraction methods.
	 */
#ifndef _NO_LONGLONG
#define	SARG() \
	(flags&QUADINT ? GET_ARG (N, ap, quad_t) : \
	    flags&LONGINT ? GET_ARG (N, ap, long) : \
	    flags&SHORTINT ? (long)(short)GET_ARG (N, ap, int) : \
	    flags&CHARINT ? (long)(signed char)GET_ARG (N, ap, int) : \
	    (long)GET_ARG (N, ap, int))
#define	UARG() \
	(flags&QUADINT ? GET_ARG (N, ap, u_quad_t) : \
	    flags&LONGINT ? GET_ARG (N, ap, u_long) : \
	    flags&SHORTINT ? (u_long)(u_short)GET_ARG (N, ap, int) : \
	    flags&CHARINT ? (u_long)(unsigned char)GET_ARG (N, ap, int) : \
	    (u_long)GET_ARG (N, ap, u_int))
#else
#define	SARG() \
	(flags&LONGINT ? GET_ARG (N, ap, long) : \
	    flags&SHORTINT ? (long)(short)GET_ARG (N, ap, int) : \
	    flags&CHARINT ? (long)(signed char)GET_ARG (N, ap, int) : \
	    (long)GET_ARG (N, ap, int))
#define	UARG() \
	(flags&LONGINT ? GET_ARG (N, ap, u_long) : \
	    flags&SHORTINT ? (u_long)(u_short)GET_ARG (N, ap, int) : \
	    flags&CHARINT ? (u_long)(unsigned char)GET_ARG (N, ap, int) : \
	    (u_long)GET_ARG (N, ap, u_int))
#endif

#ifndef STRING_ONLY
	/* Initialize std streams if not dealing with sprintf family.  */
	CHECK_INIT (data, fp);
	_newlib_flockfile_start (fp);

	if (ORIENT(fp, 1) != 1) {
		_newlib_flockfile_exit (fp);
		return (EOF);
	}

	/* sorry, fwprintf(read_only_file, "") returns EOF, not 0 */
	if (cantwrite (data, fp)) {
		_newlib_flockfile_exit (fp);
		return (EOF);
	}

#ifdef _UNBUF_STREAM_OPT
	/* optimise fwprintf(stderr) (and other unbuffered Unix files) */
	if ((fp->_flags & (__SNBF|__SWR|__SRW)) == (__SNBF|__SWR) &&
	    fp->_file >= 0) {
		_newlib_flockfile_exit (fp);
		return (__sbwprintf (fp, fmt0, ap));
	}
#endif
#else /* STRING_ONLY */
        /* Create initial buffer if we are called by asprintf family.  */
        if (fp->_flags & __SMBF && !fp->_bf._base)
        {
		fp->_bf._base = fp->_p = malloc (64);
		if (!fp->_p)
		{
			_REENT_ERRNO(data) = ENOMEM;
			return EOF;
		}
		fp->_bf._size = 64;
        }
#endif /* STRING_ONLY */

	fmt = (wchar_t *)fmt0;
#ifdef _FVWRITE_IN_STREAMIO
	uio.uio_iov = iovp = iov;
	uio.uio_resid = 0;
	uio.uio_iovcnt = 0;
#endif
	ret = 0;
#ifndef _NO_POS_ARGS
	arg_index = 0;
	saved_fmt = NULL;
	arg_type[0] = -1;
	numargs = 0;
	is_pos_arg = 0;
#endif

	/*
	 * Scan the format for conversions (`%' character).
	 */
	for (;;) {
	        cp = fmt;
                while (*fmt != L'\0' && *fmt != L'%')
                    ++fmt;
		if ((m = fmt - cp) != 0) {
			PRINT (cp, m);
			ret += m;
		}
                if (*fmt == L'\0')
                    goto done;
#ifndef _NO_POS_ARGS
		fmt_anchor = fmt;
#endif
		fmt++;		/* skip over '%' */

		flags = 0;
		dprec = 0;
		width = 0;
		prec = -1;
		sign = L'\0';
#ifdef FLOATING_POINT
		lead = 0;
#ifdef _WANT_IO_C99_FORMATS
		nseps = nrepeats = 0;
#endif
#endif
#ifndef _NO_POS_ARGS
		N = arg_index;
		is_pos_arg = 0;
#endif

rflag:		ch = *fmt++;
reswitch:	switch (ch) {
#ifdef _WANT_IO_C99_FORMATS
		case L'\'':
#ifdef _MB_CAPABLE
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
		  thousands_sep = *__get_current_numeric_locale ()->wthousands_sep;
#else
		  {
		    size_t nconv;

		    memset (&state, '\0', sizeof (state));
		    nconv = mbrtowc (&thousands_sep,
					localeconv ()->thousands_sep,
					MB_CUR_MAX, &state);
		    if (nconv == (size_t) -1 || nconv == (size_t) -2)
		      thousands_sep = L'\0';
		  }
#endif
#else
		  thousands_sep = (wchar_t) *localeconv ()->thousands_sep;
#endif
		  grouping = localeconv ()->grouping;
		  if (thousands_sep && grouping && *grouping)
		    flags |= GROUPING;
		  goto rflag;
#endif
		case L' ':
			/*
			 * ``If the space and + flags both appear, the space
			 * flag will be ignored.''
			 *	-- ANSI X3J11
			 */
			if (!sign)
				sign = L' ';
			goto rflag;
		case L'#':
			flags |= ALT;
			goto rflag;
		case L'*':
#ifndef _NO_POS_ARGS
			/* we must check for positional arg used for dynamic width */
			n = N;
			old_is_pos_arg = is_pos_arg;
			is_pos_arg = 0;
			if (is_digit (*fmt)) {
				wchar_t *old_fmt = fmt;

				n = 0;
				ch = *fmt++;
				do {
					n = 10 * n + to_digit (ch);
					ch = *fmt++;
				} while (is_digit (ch));

				if (ch == L'$') {
					if (n <= MAX_POS_ARGS) {
						n -= 1;
						is_pos_arg = 1;
					}
					else
						goto error;
				}
				else {
					fmt = old_fmt;
					goto rflag;
				}
			}
#endif /* !_NO_POS_ARGS */

			/*
			 * ``A negative field width argument is taken as a
			 * - flag followed by a positive field width.''
			 *	-- ANSI X3J11
			 * They don't exclude field widths read from args.
			 */
			width = GET_ARG (n, ap, int);
#ifndef _NO_POS_ARGS
			is_pos_arg = old_is_pos_arg;
#endif
			if (width >= 0)
				goto rflag;
			width = -width;
			__PICOLIBC_FALLTHROUGH;
		case L'-':
			flags |= LADJUST;
			goto rflag;
		case L'+':
			sign = L'+';
			goto rflag;
		case L'.':
			if ((ch = *fmt++) == L'*') {
#ifndef _NO_POS_ARGS
				/* we must check for positional arg used for dynamic width */
				n = N;
				old_is_pos_arg = is_pos_arg;
				is_pos_arg = 0;
				if (is_digit (*fmt)) {
					wchar_t *old_fmt = fmt;

					n = 0;
					ch = *fmt++;
					do {
						n = 10 * n + to_digit (ch);
						ch = *fmt++;
					} while (is_digit (ch));

					if (ch == L'$') {
						if (n <= MAX_POS_ARGS) {
							n -= 1;
							is_pos_arg = 1;
						}
						else
							goto error;
					}
					else {
						fmt = old_fmt;
						goto rflag;
					}
				}
#endif /* !_NO_POS_ARGS */
				prec = GET_ARG (n, ap, int);
#ifndef _NO_POS_ARGS
				is_pos_arg = old_is_pos_arg;
#endif
				if (prec < 0)
					prec = -1;
				goto rflag;
			}
			n = 0;
			while (is_digit (ch)) {
				n = 10 * n + to_digit (ch);
				ch = *fmt++;
			}
			prec = n < 0 ? -1 : n;
			goto reswitch;
		case L'0':
			/*
			 * ``Note that 0 is taken as a flag, not as the
			 * beginning of a field width.''
			 *	-- ANSI X3J11
			 */
			flags |= ZEROPAD;
			goto rflag;
		case L'1': case L'2': case L'3': case L'4':
		case L'5': case L'6': case L'7': case L'8': case L'9':
			n = 0;
			do {
				n = 10 * n + to_digit (ch);
				ch = *fmt++;
			} while (is_digit (ch));
#ifndef _NO_POS_ARGS
			if (ch == L'$') {
				if (n <= MAX_POS_ARGS) {
					N = n - 1;
					is_pos_arg = 1;
					goto rflag;
				}
				else
					goto error;
			}
#endif /* !_NO_POS_ARGS */
			width = n;
			goto reswitch;
#ifdef FLOATING_POINT
		case L'L':
			flags |= LONGDBL;
			goto rflag;
#endif
		case L'h':
#ifdef _WANT_IO_C99_FORMATS
			if (*fmt == L'h') {
				fmt++;
				flags |= CHARINT;
			} else
#endif
				flags |= SHORTINT;
			goto rflag;
		case L'l':
#if defined _WANT_IO_C99_FORMATS || !defined _NO_LONGLONG
			if (*fmt == L'l') {
				fmt++;
				flags |= QUADINT;
			} else
#endif
				flags |= LONGINT;
			goto rflag;
		case L'q': /* GNU extension */
			flags |= QUADINT;
			goto rflag;
#ifdef _WANT_IO_C99_FORMATS
		case L'j':
		  if (sizeof (intmax_t) == sizeof (long))
		    flags |= LONGINT;
		  else
		    flags |= QUADINT;
		  goto rflag;
		case L'z':
		  if (sizeof (size_t) < sizeof (int))
		    /* POSIX states size_t is 16 or more bits, as is short.  */
		    flags |= SHORTINT;
		  else if (sizeof (size_t) == sizeof (int))
		    /* no flag needed */;
		  else if (sizeof (size_t) <= sizeof (long))
		    flags |= LONGINT;
		  else
		    /* POSIX states that at least one programming
		       environment must support size_t no wider than
		       long, but that means other environments can
		       have size_t as wide as long long.  */
		    flags |= QUADINT;
		  goto rflag;
		case L't':
		  if (sizeof (ptrdiff_t) < sizeof (int))
		    /* POSIX states ptrdiff_t is 16 or more bits, as
		       is short.  */
		    flags |= SHORTINT;
		  else if (sizeof (ptrdiff_t) == sizeof (int))
		    /* no flag needed */;
		  else if (sizeof (ptrdiff_t) <= sizeof (long))
		    flags |= LONGINT;
		  else
		    /* POSIX states that at least one programming
		       environment must support ptrdiff_t no wider than
		       long, but that means other environments can
		       have ptrdiff_t as wide as long long.  */
		    flags |= QUADINT;
		  goto rflag;
		case L'C':	/* POSIX extension */
#endif /* _WANT_IO_C99_FORMATS */
		case L'c':
			cp = buf;
			if (ch == L'c' && !(flags & LONGINT)) {
				wint_t wc = btowc ((int) GET_ARG (N, ap, int));
				if (wc == WEOF) {
				    fp->_flags |= __SERR;
				    goto error;
				}
				cp[0] = (wchar_t) wc;
			}
			else
			{
				cp[0] = GET_ARG (N, ap, int);
			}
			cp[1] = L'\0';
			size = 1;
			sign = L'\0';
			break;
		case L'd':
		case L'i':
			_uquad = SARG ();
#ifndef _NO_LONGLONG
			if ((quad_t)_uquad < 0)
#else
			if ((long) _uquad < 0)
#endif
			{

				_uquad = -_uquad;
				sign = L'-';
			}
			base = DEC;
			goto number;
#ifdef FLOATING_POINT
# ifdef _WANT_IO_C99_FORMATS
		case L'a':
		case L'A':
		case L'F':
# endif
		case L'e':
		case L'E':
		case L'f':
		case L'g':
		case L'G':
# ifdef _NO_LONGDBL
			if (flags & LONGDBL) {
				_fpvalue = (double) GET_ARG (N, ap, _LONG_DOUBLE);
			} else {
				_fpvalue = GET_ARG (N, ap, double);
			}

			/* do this before tricky precision changes

			   If the output is infinite or NaN, leading
			   zeros are not permitted.  Otherwise, scanf
			   could not read what printf wrote.
			 */
			if (isinf (_fpvalue)) {
				if (_fpvalue < 0)
					sign = '-';
				if (ch <= L'G') /* 'A', 'E', 'F', or 'G' */
					cp = L"INF";
				else
					cp = L"inf";
				size = 3;
				flags &= ~ZEROPAD;
				break;
			}
			if (isnan (_fpvalue)) {
				if (signbit (_fpvalue))
					sign = L'-';
				if (ch <= L'G') /* 'A', 'E', 'F', or 'G' */
					cp = L"NAN";
				else
					cp = L"nan";
				size = 3;
				flags &= ~ZEROPAD;
				break;
			}

# else /* !_NO_LONGDBL */

			if (flags & LONGDBL) {
				_fpvalue = GET_ARG (N, ap, _LONG_DOUBLE);
			} else {
				_fpvalue = (_LONG_DOUBLE)GET_ARG (N, ap, double);
			}

			/* do this before tricky precision changes */
			expt = _ldcheck (&_fpvalue);
			if (expt == 2) {
				if (_fpvalue < 0)
					sign = L'-';
				if (ch <= L'G') /* 'A', 'E', 'F', or 'G' */
					cp = L"INF";
				else
					cp = L"inf";
				size = 3;
				flags &= ~ZEROPAD;
				break;
			}
			if (expt == 1) {
				if (signbit (_fpvalue))
					sign = L'-';
				if (ch <= L'G') /* 'A', 'E', 'F', or 'G' */
					cp = L"NAN";
				else
					cp = L"nan";
				size = 3;
				flags &= ~ZEROPAD;
				break;
			}
# endif /* !_NO_LONGDBL */

			cp = buf;
# ifdef _WANT_IO_C99_FORMATS
			if (ch == L'a' || ch == L'A') {
				ox[0] = L'0';
				ox[1] = ch == L'a' ? L'x' : L'X';
				flags |= HEXPREFIX;
				if (prec >= BUF)
				  {
				    if ((malloc_buf =
					 (wchar_t *)malloc ((prec + 1) * sizeof (wchar_t)))
					== NULL)
				      {
					fp->_flags |= __SERR;
					goto error;
				      }
				    cp = malloc_buf;
				  }
			} else
# endif /* _WANT_IO_C99_FORMATS */
			if (prec == -1) {
				prec = DEFPREC;
			} else if ((ch == L'g' || ch == L'G') && prec == 0) {
				prec = 1;
			}

			flags |= FPT;

			cp = wcvt (_fpvalue, prec, flags, &softsign,
				   &expt, ch, &ndig, cp, BUF);

			/* If buf is not large enough for the converted wchar_t
			   sequence, call wcvt again with a malloced new buffer.
			   This should happen fairly rarely.
			 */
			if (cp == buf && ndig > BUF && malloc_buf == NULL) {
				if ((malloc_buf =
				    (wchar_t *)malloc (ndig * sizeof (wchar_t)))
				    == NULL)
				  {
				    fp->_flags |= __SERR;
				    goto error;
				  }
				cp = wcvt (_fpvalue, prec, flags, &softsign,
					   &expt, ch, &ndig, malloc_buf, ndig);
			}

			if (ch == L'g' || ch == L'G') {
				if (expt <= -4 || expt > prec)
					ch -= 2; /* 'e' or 'E' */
				else
					ch = L'g';
			}
# ifdef _WANT_IO_C99_FORMATS
			else if (ch == L'F')
				ch = L'f';
# endif
			if (ch <= L'e') {	/* 'a', 'A', 'e', or 'E' fmt */
				--expt;
				expsize = wexponent (expstr, expt, ch);
				size = expsize + ndig;
				if (ndig > 1 || flags & ALT)
					++size;
# ifdef _WANT_IO_C99_FORMATS
				flags &= ~GROUPING;
# endif
			} else {
				if (ch == L'f') {		/* f fmt */
					if (expt > 0) {
						size = expt;
						if (prec || flags & ALT)
							size += prec + 1;
					} else	/* "0.X" */
						size = (prec || flags & ALT)
							  ? prec + 2
							  : 1;
				} else if (expt >= ndig) { /* fixed g fmt */
					size = expt;
					if (flags & ALT)
						++size;
				} else
					size = ndig + (expt > 0 ?
						1 : 2 - expt);
# ifdef _WANT_IO_C99_FORMATS
				if ((flags & GROUPING) && expt > 0) {
					/* space for thousands' grouping */
					nseps = nrepeats = 0;
					lead = expt;
					while (*grouping != CHAR_MAX) {
						if (lead <= *grouping)
							break;
						lead -= *grouping;
						if (grouping[1]) {
							nseps++;
							grouping++;
						} else
							nrepeats++;
					}
					size += nseps + nrepeats;
				} else
# endif
				lead = expt;
			}
			if (softsign)
				sign = L'-';
			break;
#endif /* FLOATING_POINT */
#ifdef _GLIBC_EXTENSION
		case L'm':  /* GNU extension */
			{
				int dummy;
				cp = (wchar_t *) strerror ( _REENT_ERRNO(data), 1, &dummy);
			}
			flags &= ~LONGINT;
			goto string;
#endif
		case L'n':
#ifndef _NO_LONGLONG
			if (flags & QUADINT)
				*GET_ARG (N, ap, quad_ptr_t) = ret;
			else
#endif
			if (flags & LONGINT)
				*GET_ARG (N, ap, long_ptr_t) = ret;
			else if (flags & SHORTINT)
				*GET_ARG (N, ap, short_ptr_t) = ret;
#ifdef _WANT_IO_C99_FORMATS
			else if (flags & CHARINT)
				*GET_ARG (N, ap, char_ptr_t) = ret;
#endif
			else
				*GET_ARG (N, ap, int_ptr_t) = ret;
			continue;	/* no output */
		case L'o':
			_uquad = UARG ();
			base = OCT;
#ifdef _WANT_IO_C99_FORMATS
			flags &= ~GROUPING;
#endif
			goto nosign;
		case L'p':
			/*
			 * ``The argument shall be a pointer to void.  The
			 * value of the pointer is converted to a sequence
			 * of printable characters, in an implementation-
			 * defined manner.''
			 *	-- ANSI X3J11
			 */
			/* NOSTRICT */
			_uquad = (uintptr_t) GET_ARG (N, ap, void_ptr_t);
			base = HEX;
			xdigs = L"0123456789abcdef";
			flags |= HEXPREFIX;
			ox[0] = L'0';
			ox[1] = ch = L'x';
			goto nosign;
		case L's':
#ifdef _WANT_IO_C99_FORMATS
		case L'S':	/* POSIX extension */
#endif
			cp = GET_ARG (N, ap, wchar_ptr_t);
#ifdef _GLIBC_EXTENSION
string:
#endif
			sign = '\0';
#ifndef __OPTIMIZE_SIZE__
			/* Behavior is undefined if the user passed a
			   NULL string when precision is not 0.
			   However, if we are not optimizing for size,
			   we might as well mirror glibc behavior.  */
			if (cp == NULL) {
				cp = L"(null)";
				size = ((unsigned) prec > 6U) ? 6 : prec;
			}
			else
#endif /* __OPTIMIZE_SIZE__ */
#ifdef _MB_CAPABLE
			if (ch != L'S' && !(flags & LONGINT)) {
				char *arg = (char *) cp;
				size_t insize = 0, nchars = 0, nconv = 0;
				mbstate_t ps;
				wchar_t *p;

				if (prec >= 0) {
					char *p = arg;
					memset ((void *)&ps, '\0', sizeof (mbstate_t));
					while (nchars < (size_t)prec) {
						nconv = mbrlen (p, MB_CUR_MAX, &ps);
						if (nconv == 0 || nconv == (size_t)-1 ||
						    nconv == (size_t)-2)
							break;
						p += nconv;
						++nchars;
						insize += nconv;
					}
					if (nconv == (size_t) -1 || nconv == (size_t) -2) {
						fp->_flags |= __SERR;
						goto error;
					}
				} else
					insize = strlen(arg);
				if (insize >= BUF) {
				    if ((malloc_buf = (wchar_t *) malloc ((insize + 1) * sizeof (wchar_t)))
					== NULL) {
						fp->_flags |= __SERR;
						goto error;
					}
					cp = malloc_buf;
				} else
					cp = buf;
				memset ((void *)&ps, '\0', sizeof (mbstate_t));
				p = cp;
				while (insize != 0) {
					nconv = mbrtowc (p, arg, insize, &ps);
					if (nconv == 0 || nconv == (size_t)-1 || nconv == (size_t)-2)
						break;
					++p;
					arg += nconv;
					insize -= nconv;
				}
				if (nconv == (size_t) -1 || nconv == (size_t) -2) {
					fp->_flags |= __SERR;
					goto error;
				}
				*p = L'\0';
				size = p - cp;
			}
#else
			if (ch != L'S' && !(flags & LONGINT)) {
				char *arg = (char *) cp;
				size_t insize = 0;

				if (prec >= 0) {
					char *p = memchr (arg, '\0', prec);
					insize = p ? p - arg : prec;
				} else
					insize = strlen (arg);
				if (insize >= BUF) {
				    if ((malloc_buf = (wchar_t *) malloc ((insize + 1) * sizeof (wchar_t)))
					== NULL) {
						fp->_flags |= __SERR;
						goto error;
					}
					cp = malloc_buf;
				} else
					cp = buf;
				for (size = 0; (size_t) size < insize; ++size)
					cp[size] = arg[size];
				cp[size] = L'\0';
			}
#endif /* _MB_CAPABLE */
			else if (prec >= 0) {
				/*
				 * can't use wcslen; can only look for the
				 * NUL in the first `prec' characters, and
				 * strlen () will go further.
				 */
				wchar_t *p = wmemchr (cp, L'\0', prec);

				if (p != NULL) {
					size = p - cp;
					if (size > prec)
						size = prec;
				} else
					size = prec;
			} else
				size = wcslen (cp);

			break;
		case L'u':
			_uquad = UARG ();
			base = DEC;
			goto nosign;
		case L'X':
			xdigs = L"0123456789ABCDEF";
			goto hex;
		case L'x':
			xdigs = L"0123456789abcdef";
hex:			_uquad = UARG ();
			base = HEX;
			/* leading 0x/X only if non-zero */
			if (flags & ALT && _uquad != 0) {
				ox[0] = L'0';
				ox[1] = ch;
				flags |= HEXPREFIX;
			}

#ifdef _WANT_IO_C99_FORMATS
			flags &= ~GROUPING;
#endif
			/* unsigned conversions */
nosign:			sign = L'\0';
			/*
			 * ``... diouXx conversions ... if a precision is
			 * specified, the 0 flag will be ignored.''
			 *	-- ANSI X3J11
			 */
number:			if ((dprec = prec) >= 0)
				flags &= ~ZEROPAD;

			/*
			 * ``The result of converting a zero value with an
			 * explicit precision of zero is no characters.''
			 *	-- ANSI X3J11
			 */
			cp = buf + BUF;
			if (_uquad != 0 || prec != 0) {
				/*
				 * Unsigned mod is hard, and unsigned mod
				 * by a constant is easier than that by
				 * a variable; hence this switch.
				 */
				switch (base) {
				case OCT:
					do {
						*--cp = to_char (_uquad & 7);
						_uquad >>= 3;
					} while (_uquad);
					/* handle octal leading 0 */
					if (flags & ALT && *cp != L'0')
						*--cp = L'0';
					break;

				case DEC:
					/* many numbers are 1 digit */
					if (_uquad < 10) {
						*--cp = to_char(_uquad);
						break;
					}
#ifdef _WANT_IO_C99_FORMATS
					ndig = 0;
#endif
					do {
					  *--cp = to_char (_uquad % 10);
#ifdef _WANT_IO_C99_FORMATS
					  ndig++;
					  /* If (*grouping == CHAR_MAX) then no
					     more grouping */
					  if ((flags & GROUPING)
					      && ndig == *grouping
					      && *grouping != CHAR_MAX
					      && _uquad > 9) {
					    *--cp = thousands_sep;
					    ndig = 0;
					    /* If (grouping[1] == '\0') then we
					       have to use *grouping character
					       (last grouping rule) for all
					       next cases. */
					    if (grouping[1] != '\0')
					      grouping++;
					  }
#endif
					  _uquad /= 10;
					} while (_uquad != 0);
					break;

				case HEX:
					do {
						*--cp = xdigs[_uquad & 15];
						_uquad >>= 4;
					} while (_uquad);
					break;

				default:
					cp = L"bug in vfprintf: bad base";
					size = wcslen (cp);
					goto skipsize;
				}
			}
                       /*
			* ...result is to be converted to an 'alternate form'.
			* For o conversion, it increases the precision to force
			* the first digit of the result to be a zero."
			*     -- ANSI X3J11
			*
			* To demonstrate this case, compile and run:
                        *    printf ("%#.0o",0);
			*/
                       else if (base == OCT && (flags & ALT))
                         *--cp = L'0';

			size = buf + BUF - cp;
		skipsize:
			break;
		default:	/* "%?" prints ?, unless ? is NUL */
			if (ch == L'\0')
				goto done;
			/* pretend it was %c with argument ch */
			cp = buf;
			*cp = ch;
			size = 1;
			sign = L'\0';
			break;
		}

		/*
		 * All reasonable formats wind up here.  At this point, `cp'
		 * points to a string which (if not flags&LADJUST) should be
		 * padded out to `width' places.  If flags&ZEROPAD, it should
		 * first be prefixed by any sign or other prefix; otherwise,
		 * it should be blank padded before the prefix is emitted.
		 * After any left-hand padding and prefixing, emit zeroes
		 * required by a decimal [diouxX] precision, then print the
		 * string proper, then emit zeroes required by any leftover
		 * floating precision; finally, if LADJUST, pad with blanks.
		 * If flags&FPT, ch must be in [aAeEfg].
		 *
		 * Compute actual size, so we know how much to pad.
		 * size excludes decimal prec; realsz includes it.
		 */
		realsz = dprec > size ? dprec : size;
		if (sign)
			realsz++;
		if (flags & HEXPREFIX)
			realsz+= 2;

		/* right-adjusting blank padding */
		if ((flags & (LADJUST|ZEROPAD)) == 0)
			PAD (width - realsz, blanks);

		/* prefix */
		if (sign)
			PRINT (&sign, 1);
		if (flags & HEXPREFIX)
			PRINT (ox, 2);

		/* right-adjusting zero padding */
		if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
			PAD (width - realsz, zeroes);

		/* leading zeroes from decimal precision */
		PAD (dprec - size, zeroes);

		/* the string or number proper */
#ifdef FLOATING_POINT
		if ((flags & FPT) == 0) {
			PRINT (cp, size);
		} else {	/* glue together f_p fragments */
			if (ch >= L'f') {	/* 'f' or 'g' */
				if (_fpvalue == 0) {
					/* kludge for __dtoa irregularity */
					PRINT (L"0", 1);
					if (expt < ndig || flags & ALT) {
						PRINT (&decimal_point, 1);
						PAD (ndig - 1, zeroes);
					}
				} else if (expt <= 0) {
					PRINT (L"0", 1);
					if (expt || ndig || flags & ALT) {
						PRINT (&decimal_point, 1);
						PAD (-expt, zeroes);
						PRINT (cp, ndig);
					}
				} else {
					wchar_t *convbuf = cp;
					PRINTANDPAD(cp, convbuf + ndig,
						    lead, zeroes);
					cp += lead;
#ifdef _WANT_IO_C99_FORMATS
					if (flags & GROUPING) {
					    while (nseps > 0 || nrepeats > 0) {
						if (nrepeats > 0)
						    nrepeats--;
						else {
						    grouping--;
						    nseps--;
						}
						PRINT (&thousands_sep, 1);
						PRINTANDPAD (cp, convbuf + ndig,
							     *grouping, zeroes);
						cp += *grouping;
					    }
					    if (cp > convbuf + ndig)
						cp = convbuf + ndig;
					}
#endif
					if (expt < ndig || flags & ALT)
					    PRINT (&decimal_point, 1);
					PRINTANDPAD (cp, convbuf + ndig,
						     ndig - expt, zeroes);
				}

			} else {	/* 'a', 'A', 'e', or 'E' */
				if (ndig > 1 || flags & ALT) {
					PRINT (cp, 1);
					cp++;
					PRINT (&decimal_point, 1);
					if (_fpvalue) {
						PRINT (cp, ndig - 1);
					} else	/* 0.[0..] */
						/* __dtoa irregularity */
						PAD (ndig - 1, zeroes);
				} else	/* XeYYY */
					PRINT (cp, 1);
				PRINT (expstr, expsize);
			}
		}
#else /* !FLOATING_POINT */
		PRINT (cp, size);
#endif
		/* left-adjusting padding (always blank) */
		if (flags & LADJUST)
			PAD (width - realsz, blanks);

		/* finally, adjust ret */
		ret += width > realsz ? width : realsz;

		FLUSH ();	/* copy out the I/O vectors */

                if (malloc_buf != NULL) {
			free (malloc_buf);
			malloc_buf = NULL;
		}
	}
done:
	FLUSH ();
error:
	if (malloc_buf != NULL)
		free (malloc_buf);
#ifndef STRING_ONLY
	_newlib_flockfile_end (fp);
#endif
	return (__sferror (fp) ? EOF : ret);
	/* NOTREACHED */
}

#ifdef FLOATING_POINT

/* Convert finite VALUE into a string of digits
   with no decimal point, using NDIGITS precision and FLAGS as guides
   to whether trailing zeros must be included.  Set *SIGN to nonzero
   if VALUE was negative.  Set *DECPT to the exponent plus one.  Set
   *LENGTH to the length of the returned string.  CH must be one of
   [aAeEfFgG]; different from vfprintf.c:cvt(), the return string
   lives in BUF regardless of CH.  LEN is the length of BUF, except
   when CH is [aA], in which case LEN is not in use.  If BUF is not
   large enough for the converted string, only the first LEN number
   of characters will be returned in BUF, but *LENGTH will be set to
   the full length of the string before the truncation.  */
static wchar_t *
wcvt(_PRINTF_FLOAT_TYPE value, int ndigits, int flags,
     wchar_t *sign, int *decpt, int ch, int *length, wchar_t *buf, int len)
{
	int mode, dsgn;
# ifdef _NO_LONGDBL
	union double_union tmp;

	tmp.d = value;
	if (word0 (tmp) & Sign_bit) { /* this will check for < 0 and -0.0 */
		value = -value;
		*sign = L'-';
	} else
		*sign = L'\0';
# else /* !_NO_LONGDBL */
	union
	{
	  struct ldieee ieee;
	  _LONG_DOUBLE val;
	} ld;

	ld.val = value;
	if (ld.ieee.sign) { /* this will check for < 0 and -0.0 */
		value = -value;
		*sign = L'-';
	} else
		*sign = L'\0';
# endif /* !_NO_LONGDBL */

# ifdef _WANT_IO_C99_FORMATS
	if (ch == L'a' || ch == L'A') {
		wchar_t *digits, *bp, *rve;
		/* This code assumes FLT_RADIX is a power of 2.  The initial
		   division ensures the digit before the decimal will be less
		   than FLT_RADIX (unless it is rounded later).	 There is no
		   loss of precision in these calculations.  */
		value = FREXP (value, decpt) / 8;
		if (!value)
			*decpt = 1;
		digits = ch == L'a' ? L"0123456789abcdef" : L"0123456789ABCDEF";
		bp = buf;
		do {
			value *= 16;
			mode = (int) value;
			value -= mode;
			*bp++ = digits[mode];
		} while (ndigits-- && value);
		if (value > (_PRINTF_FLOAT_TYPE) 0.5 || (value == (_PRINTF_FLOAT_TYPE) 0.5 && mode & 1)) {
			/* round to even */
			rve = bp;
			while (*--rve == digits[0xf]) {
				*rve = L'0';
			}
			*rve = *rve == L'9' ? digits[0xa] : *rve + 1;
		} else {
			while (ndigits-- >= 0) {
				*bp++ = L'0';
			}
		}
		*length = bp - buf;
		return buf;
	}
# endif /* _WANT_IO_C99_FORMATS */
	if (ch == L'f' || ch == L'F') {
		mode = 3;		/* ndigits after the decimal point */
	} else {
		/* To obtain ndigits after the decimal point for the 'e'
		 * and 'E' formats, round to ndigits + 1 significant
		 * figures.
		 */
		if (ch == L'e' || ch == L'E') {
			ndigits++;
		}
		mode = 2;		/* ndigits significant digits */
	}

	{
	  char *digits, *bp, *rve;
#ifndef _MB_CAPABLE
	  int i;
#endif

	  digits = _DTOA (value, mode, ndigits, decpt, &dsgn, &rve);

	  if ((ch != L'g' && ch != L'G') || flags & ALT) {	/* Print trailing zeros */
		bp = digits + ndigits;
		if (ch == L'f' || ch == L'F') {
			if (*digits == L'0' && value)
				*decpt = -ndigits + 1;
			bp += *decpt;
		}
		if (value == 0)	/* kludge for __dtoa irregularity */
			rve = bp;
		while (rve < bp)
			*rve++ = '0';
	  }

	  *length = rve - digits; /* full length of the string */
#ifdef _MB_CAPABLE
	  mbsnrtowcs (buf, (const char **) &digits, *length,
			 len, NULL);
#else
	  for (i = 0; i < *length && i < len; ++i)
	    buf[i] = (wchar_t) digits[i];
#endif
	  return buf;
	}
}

static int
wexponent(wchar_t *p0, int exp, int fmtch)
{
	register wchar_t *p, *t;
	wchar_t expbuf[MAXEXPLEN];
# ifdef _WANT_IO_C99_FORMATS
	int isa = fmtch == L'a' || fmtch == L'A';
# else
#  define isa 0
# endif

	p = p0;
	*p++ = isa ? (int) (L'p' - L'a') + fmtch : fmtch;
	if (exp < 0) {
		exp = -exp;
		*p++ = L'-';
	}
	else
		*p++ = L'+';
	t = expbuf + MAXEXPLEN;
	if (exp > 9) {
		do {
			*--t = to_char (exp % 10);
		} while ((exp /= 10) > 9);
		*--t = to_char (exp);
		for (; t < expbuf + MAXEXPLEN; *p++ = *t++);
	}
	else {
		if (!isa)
			*p++ = L'0';
		*p++ = to_char (exp);
	}
	return (p - p0);
}
#endif /* FLOATING_POINT */


#ifndef _NO_POS_ARGS

/* Positional argument support.
   Written by Jeff Johnston

   Copyright (c) 2002 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

      Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

      The name of Red Hat Incorporated may not be used to endorse
      or promote products derived from this software without specific
      prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

/* function to get positional parameter N where n = N - 1 */
static union arg_val *
get_arg (
       int n,
       wchar_t *fmt,
       my_va_list *ap,
       int *numargs_p,
       union arg_val *args,
       int *arg_type,
       wchar_t **last_fmt)
{
  wchar_t ch;
  int number, flags;
  int spec_type;
  int numargs = *numargs_p;
  __CH_CLASS chtype;
  __STATE state, next_state;
  __ACTION action;
  int pos, last_arg;
  int max_pos_arg = n;
  /* Only need types that can be reached via vararg promotions.  */
  enum types { INT, LONG_INT, QUAD_INT, CHAR_PTR, DOUBLE, LONG_DOUBLE, WIDE_CHAR };

  /* if this isn't the first call, pick up where we left off last time */
  if (*last_fmt != NULL)
    fmt = *last_fmt;

  /* we need to process either to end of fmt string or until we have actually
     read the desired parameter from the vararg list. */
  while (*fmt && n >= numargs)
    {
      while (*fmt != L'\0' && *fmt != L'%')
				fmt += 1;

      if (*fmt == L'\0')
				break;
      state = START;
      flags = 0;
      pos = -1;
      number = 0;
      spec_type = INT;

      /* Use state/action table to process format specifiers.  We ignore invalid
         formats and we are only interested in information that tells us how to
         read the vararg list. */
      while (state != DONE)
	{
	  ch = *fmt++;
	  chtype = ch < (wchar_t) 256 ? __chclass[ch] : OTHER;
	  next_state = __state_table[state][chtype];
	  action = __action_table[state][chtype];
	  state = next_state;

	  switch (action)
	    {
	    case GETMOD:  /* we have format modifier */
	      switch (ch)
		{
		case L'h':
		  /* No flag needed, since short and char promote to int.  */
		  break;
		case L'L':
		  flags |= LONGDBL;
		  break;
		case L'q':
		  flags |= QUADINT;
		  break;
# ifdef _WANT_IO_C99_FORMATS
		case L'j':
		  if (sizeof (intmax_t) == sizeof (long))
		    flags |= LONGINT;
		  else
		    flags |= QUADINT;
		  break;
		case L'z':
		  if (sizeof (size_t) <= sizeof (int))
		    /* no flag needed */;
		  else if (sizeof (size_t) <= sizeof (long))
		    flags |= LONGINT;
		  else
		    /* POSIX states that at least one programming
		       environment must support size_t no wider than
		       long, but that means other environments can
		       have size_t as wide as long long.  */
		    flags |= QUADINT;
		  break;
		case L't':
		  if (sizeof (ptrdiff_t) <= sizeof (int))
		    /* no flag needed */;
		  else if (sizeof (ptrdiff_t) <= sizeof (long))
		    flags |= LONGINT;
		  else
		    /* POSIX states that at least one programming
		       environment must support ptrdiff_t no wider than
		       long, but that means other environments can
		       have ptrdiff_t as wide as long long.  */
		    flags |= QUADINT;
		  break;
# endif /* _WANT_IO_C99_FORMATS */
		case L'l':
		default:
# if defined _WANT_IO_C99_FORMATS || !defined _NO_LONGLONG
		  if (*fmt == L'l')
		    {
		      flags |= QUADINT;
		      ++fmt;
		    }
		  else
# endif
		    flags |= LONGINT;
		  break;
		}
	      break;
	    case GETARG: /* we have format specifier */
	      {
		numargs &= (MAX_POS_ARGS - 1);
		/* process the specifier and translate it to a type to fetch from varargs */
		switch (ch)
		  {
		  case L'd':
		  case L'i':
		  case L'o':
		  case L'x':
		  case L'X':
		  case L'u':
		    if (flags & LONGINT)
		      spec_type = LONG_INT;
# ifndef _NO_LONGLONG
		    else if (flags & QUADINT)
		      spec_type = QUAD_INT;
# endif
		    else
		      spec_type = INT;
		    break;
# ifdef _WANT_IO_C99_FORMATS
		  case L'a':
		  case L'A':
		  case L'F':
# endif
		  case L'f':
		  case L'g':
		  case L'G':
		  case L'E':
		  case L'e':
# ifndef _NO_LONGDBL
		    if (flags & LONGDBL)
		      spec_type = LONG_DOUBLE;
		    else
# endif
		      spec_type = DOUBLE;
		    break;
		  case L's':
# ifdef _WANT_IO_C99_FORMATS
		  case L'S':	/* POSIX extension */
# endif
		  case L'p':
		  case L'n':
		    spec_type = CHAR_PTR;
		    break;
		  case L'c':
# ifdef _WANT_IO_C99_FORMATS
		    if (flags & LONGINT)
		      spec_type = WIDE_CHAR;
		    else
# endif
		      spec_type = INT;
		    break;
# ifdef _WANT_IO_C99_FORMATS
		  case L'C':	/* POSIX extension */
		    spec_type = WIDE_CHAR;
		    break;
# endif
		  }

		/* if we have a positional parameter, just store the type, otherwise
		   fetch the parameter from the vararg list */
		if (pos != -1)
		  arg_type[pos] = spec_type;
		else
		  {
		    switch (spec_type)
		      {
		      case LONG_INT:
			args[numargs++].val_long = va_arg (ap->ap, long);
			break;
		      case QUAD_INT:
			args[numargs++].val_quad_t = va_arg (ap->ap, quad_t);
			break;
		      case WIDE_CHAR:
			args[numargs++].val_wint_t = va_arg (ap->ap, wint_t);
			break;
		      case INT:
			args[numargs++].val_int = va_arg (ap->ap, int);
			break;
		      case CHAR_PTR:
			args[numargs++].val_wchar_ptr_t = va_arg (ap->ap, wchar_t *);
			break;
		      case DOUBLE:
			args[numargs++].val_double = va_arg (ap->ap, double);
			break;
		      case LONG_DOUBLE:
			args[numargs++].val__LONG_DOUBLE = va_arg (ap->ap, _LONG_DOUBLE);
			break;
		      }
		  }
	      }
	      break;
	    case GETPOS: /* we have positional specifier */
	      if (arg_type[0] == -1)
		memset (arg_type, 0, sizeof (int) * MAX_POS_ARGS);
	      pos = number - 1;
	      max_pos_arg = (max_pos_arg > pos ? max_pos_arg : pos);
	      break;
	    case PWPOS:  /* we have positional specifier for width or precision */
	      if (arg_type[0] == -1)
		memset (arg_type, 0, sizeof (int) * MAX_POS_ARGS);
	      number -= 1;
	      arg_type[number] = INT;
	      max_pos_arg = (max_pos_arg > number ? max_pos_arg : number);
	      break;
	    case GETPWB: /* we require format pushback */
	      --fmt;
	      __PICOLIBC_FALLTHROUGH;
	    case GETPW:  /* we have a variable precision or width to acquire */
	      args[numargs++].val_int = va_arg (ap->ap, int);
	      break;
	    case NUMBER: /* we have a number to process */
	      number = (ch - '0');
	      while ((ch = *fmt) != '\0' && is_digit (ch))
		{
		  number = number * 10 + (ch - '0');
		  ++fmt;
		}
	      break;
	    case SKIPNUM: /* we have a number to skip */
	      while ((ch = *fmt) != '\0' && is_digit (ch))
		++fmt;
	      break;
	    case NOOP:
	    default:
	      break; /* do nothing */
	    }
	}
    }

  /* process all arguments up to at least the one we are looking for and if we
     have seen the end of the string, then process up to the max argument needed */
  if (*fmt == '\0')
    last_arg = max_pos_arg;
  else
    last_arg = n;

  while (numargs <= last_arg)
    {
      switch (arg_type[numargs])
	{
	case LONG_INT:
	  args[numargs++].val_long = va_arg (ap->ap, long);
	  break;
	case QUAD_INT:
	  args[numargs++].val_quad_t = va_arg (ap->ap, quad_t);
	  break;
	case CHAR_PTR:
	  args[numargs++].val_wchar_ptr_t = va_arg (ap->ap, wchar_t *);
	  break;
	case DOUBLE:
	  args[numargs++].val_double = va_arg (ap->ap, double);
	  break;
	case LONG_DOUBLE:
	  args[numargs++].val__LONG_DOUBLE = va_arg (ap->ap, _LONG_DOUBLE);
	  break;
	case WIDE_CHAR:
	  args[numargs++].val_wint_t = va_arg (ap->ap, wint_t);
	  break;
	case INT:
	default:
	  args[numargs++].val_wchar_ptr_t = va_arg (ap->ap, wchar_t *);
	  break;
	}
    }

  /* alter the global numargs value and keep a reference to the last bit of the fmt
     string we processed here because the caller will continue processing where we started */
  *numargs_p = numargs;
  *last_fmt = fmt;
  return &args[n];
}
#endif /* !_NO_POS_ARGS */
