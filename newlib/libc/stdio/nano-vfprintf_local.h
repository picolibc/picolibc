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
 * 3. Neither the name of the University nor the names of its contributors
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
 * Copyright (c) 2012-2014 ARM Ltd
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the company may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef VFPRINTF_LOCAL
#define VFPRINTF_LOCAL

#ifndef NO_FLOATING_POINT
# define FLOATING_POINT
#endif

#define _NO_POS_ARGS
#undef _WANT_IO_C99_FORMATS

/* Currently a test is made to see if long double processing is warranted.
   This could be changed in the future should the _ldtoa_r code be
   preferred over _dtoa_r.  */
#define _NO_LONGDBL

#define _NO_LONGLONG

#define _PRINTF_FLOAT_TYPE double

#if defined (FLOATING_POINT)
# include <locale.h>
#endif
#ifdef FLOATING_POINT
# include <math.h>

/* For %La, an exponent of 15 bits occupies the exponent character,
   a sign, and up to 5 digits.  */
# define MAXEXPLEN		7
# define DEFPREC		6

extern char *_dtoa_r (struct _reent *, double, int,
			      int, int *, int *, char **);

# define _DTOA_R _dtoa_r
# define FREXP frexp

#endif /* FLOATING_POINT.  */

/* BUF must be big enough for the maximum %#llo (assuming long long is
   at most 64 bits, this would be 23 characters), the maximum
   multibyte character %C, and the maximum default precision of %La
   (assuming long double is at most 128 bits with 113 bits of
   mantissa, this would be 29 characters).  %e, %f, and %g use
   reentrant storage shared with mprec.  All other formats that use
   buf get by with fewer characters.  Making BUF slightly bigger
   reduces the need for malloc in %.*a and %S, when large precision or
   long strings are processed.
   The bigger size of 100 bytes is used on systems which allow number
   strings using the locale's grouping character.  Since that's a multibyte
   value, we should use a conservative value.  */
#define	BUF		40

#define quad_t long
#define u_quad_t unsigned long

typedef quad_t * quad_ptr_t;
typedef void *void_ptr_t;
typedef char *   char_ptr_t;
typedef long *   long_ptr_t;
typedef int  *   int_ptr_t;
typedef short *  short_ptr_t;

/* Macros for converting digits to letters and vice versa.  */
#define	to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit (c) <= 9)
#define	to_char(n)	((n) + '0')

/* Flags used during conversion.  */
#define	ALT		0x001		/* Alternate form.  */
#define	LADJUST		0x002		/* Left adjustment.  */
#define	ZEROPAD		0x004		/* Zero (as opposed to blank) pad.  */
#define PLUSSGN		0x008		/* Plus sign flag.  */
#define SPACESGN	0x010		/* Space flag.  */
#define	HEXPREFIX	0x020		/* Add 0x or 0X prefix.  */
#define	SHORTINT	0x040		/* Short integer.  */
#define	LONGINT		0x080		/* Long integer.  */
#define	LONGDBL		0x100		/* Long double.  */
/* ifdef _NO_LONGLONG, make QUADINT equivalent to LONGINT, so
   that %lld behaves the same as %ld, not as %d, as expected if:
   sizeof (long long) = sizeof long > sizeof int.  */
#define QUADINT		LONGINT
#define FPT		0x400		/* Floating point number.  */
/* Define as 0, to make SARG and UARG occupy fewer instructions.  */
# define CHARINT	0

/* Macros to support positional arguments.  */
#define GET_ARG(n, ap, type) (va_arg ((ap), type))

/* To extend shorts properly, we need both signed and unsigned
   argument extraction methods.  Also they should be used in nano-vfprintf_i.c
   and nano-vfprintf_float.c only, since ap is a pointer to va_list.  */
#define	SARG(flags) \
	(flags&LONGINT ? GET_ARG (N, (*ap), long) : \
	    flags&SHORTINT ? (long)(short)GET_ARG (N, (*ap), int) : \
	    flags&CHARINT ? (long)(signed char)GET_ARG (N, (*ap), int) : \
	    (long)GET_ARG (N, (*ap), int))
#define	UARG(flags) \
	(flags&LONGINT ? GET_ARG (N, (*ap), u_long) : \
	    flags&SHORTINT ? (u_long)(u_short)GET_ARG (N, (*ap), int) : \
	    flags&CHARINT ? (u_long)(unsigned char)GET_ARG (N, (*ap), int) : \
	    (u_long)GET_ARG (N, (*ap), u_int))

/* BEWARE, these `goto error' on error. And they are used
   in more than one functions.

   Following macros are each referred about twice in printf for integer,
   so it is not worth to rewrite them into functions. This situation may
   change in the future.  */
#define PRINT(ptr, len) {		\
	if (pfunc (data, fp, (ptr), (len)) == EOF) \
		goto error;		\
}
#define PAD(howmany, ch) {             \
       int temp_i = 0;                 \
       while (temp_i < (howmany))      \
       {                               \
               if (pfunc (data, fp, &(ch), 1) == EOF) \
                       goto error;     \
               temp_i++;               \
       }			       \
}
#define PRINTANDPAD(p, ep, len, ch) {  \
       int temp_n = (ep) - (p);        \
       if (temp_n > (len))             \
               temp_n = (len);         \
       if (temp_n > 0)                 \
               PRINT((p), temp_n);     \
       PAD((len) - (temp_n > 0 ? temp_n : 0), (ch)); \
}

/* All data needed to decode format string are kept in below struct.  */
struct _prt_data_t
{
  int flags;		/* Flags.  */
  int prec;		/* Precision.  */
  int dprec;		/* Decimal precision.  */
  int width;		/* Width.  */
  int size;		/* Size of converted field or string.  */
  int ret;		/* Return value accumulator.  */
  char code;		/* Current conversion specifier.  */
  char blank;		/* Blank character.  */
  char zero;		/* Zero character.  */
  char buf[BUF];	/* Output buffer for non-floating point number.  */
  char l_buf[3];	/* Sign&hex_prefix, "+/-" and "0x/X".  */
#ifdef FLOATING_POINT
  _PRINTF_FLOAT_TYPE _double_;	/* Double value.  */
  char expstr[MAXEXPLEN];	/* Buffer for exponent string.  */
  int lead;		/* The sig figs before decimal or group sep.  */
#endif
};

extern int
_printf_common (struct _reent *data,
		struct _prt_data_t *pdata,
		int *realsz,
		FILE *fp,
		int (*pfunc)(struct _reent *, FILE *,
			     const char *, size_t len));

extern int
_printf_i (struct _reent *data, struct _prt_data_t *pdata, FILE *fp,
	   int (*pfunc)(struct _reent *, FILE *, const char *, size_t len),
	   va_list *ap);

/* Make _printf_float weak symbol, so it won't be linked in if target program
   does not need it.  */
extern int
_printf_float (struct _reent *data,
	       struct _prt_data_t *pdata,
	       FILE *fp,
	       int (*pfunc)(struct _reent *, FILE *,
			    const char *, size_t len),
	       va_list *ap) _ATTRIBUTE((__weak__));
#endif
