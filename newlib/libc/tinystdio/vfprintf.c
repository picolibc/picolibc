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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdio_private.h"

#ifdef PICOLIBC_FLOAT_PRINTF_SCANF
static inline float
printf_float_get(uint32_t u)
{
	union {
		float		f;
		uint32_t	u;
	} un = { .u = u };
	return un.f;
}

#define PRINTF_FLOAT_ARG(ap) (printf_float_get(va_arg(ap, uint32_t)))
typedef float printf_float_t;

#define dtoa ftoa
#define DTOA_MINUS FTOA_MINUS
#define DTOA_ZERO  FTOA_ZERO
#define DTOA_INF   FTOA_INF
#define DTOA_NAN   FTOA_NAN
#define DTOA_CARRY FTOA_CARRY
#define DTOA_MAX_DIG FTOA_MAX_DIG
#define __dtoa_engine(x,dtoa,dig,dec) __ftoa_engine(x,dtoa,dig,dec)
#include "ftoa_engine.h"

#else

#define PRINTF_FLOAT_ARG(ap) va_arg(ap, double)
typedef double printf_float_t;
#include "dtoa_engine.h"

#endif

/*
 * This file can be compiled into more than one flavour.  The default
 * is to offer the usual modifiers and integer formatting support
 * (level 2).  Level 1 maintains a minimal version that just offers
 * integer formatting, but no modifier support whatsoever.  Level 3 is
 * intented for floating point support.
 */

#ifndef PRINTF_LEVEL
# define PRINTF_LEVEL PRINTF_FLT
#endif

#if PRINTF_LEVEL == PRINTF_MIN || PRINTF_LEVEL == PRINTF_STD \
    || PRINTF_LEVEL == PRINTF_FLT
/* OK */
#else
# error "Not a known printf level."
#endif

#ifndef PRINTF_LONGLONG
# define PRINTF_LONGLONG	(PRINTF_LEVEL >= PRINTF_FLT)
#endif

#if PRINTF_LONGLONG
typedef unsigned long long ultoa_unsigned_t;
typedef long long ultoa_signed_t;
#define PRINTF_BUF_SIZE 22
#define arg_to_t(flags, _s_)	({				\
	    _s_ long long __v__;				\
	    if ((flags) & FL_LONG) {				\
		if ((flags) & FL_REPD_TYPE)			\
		    __v__ = va_arg(ap, _s_ long long);		\
		else						\
		    __v__ = va_arg(ap, _s_ long);		\
	    } else if ((flags) & FL_SHORT) {			\
		if ((flags) & FL_REPD_TYPE)			\
		    __v__ = (_s_ char) va_arg(ap, _s_ int);	\
		else						\
		    __v__ = (_s_ short) va_arg(ap, _s_ int);	\
	    } else {						\
		__v__ = va_arg(ap, _s_ int);			\
	    }							\
	    __v__;						\
	})
#else
typedef unsigned long ultoa_unsigned_t;
typedef long ultoa_signed_t;
#define PRINTF_BUF_SIZE 11
#define arg_to_t(flags, _s_)	({				\
	    _s_ long __v__;					\
	    if ((flags) & FL_LONG) {				\
		__v__ = va_arg(ap, _s_ long);			\
	    } else if ((flags) & FL_SHORT) {			\
		__v__ = (_s_ short) va_arg(ap, _s_ int);	\
	    } else {						\
		__v__ = va_arg(ap, _s_ int);			\
	    }							\
	    __v__;						\
	})
#endif

#define arg_to_unsigned(flags) arg_to_t(flags, unsigned)
#define arg_to_signed(flags) arg_to_t(flags, signed)

#include "ultoa_invert.c"

/* --------------------------------------------------------------------	*/
#if  PRINTF_LEVEL <= PRINTF_MIN

#define FL_ALTHEX	0x04
#define FL_ALT		0x10
#define FL_ALTLWR	0x20
#define FL_NEGATIVE	0x40
#define FL_LONG 	0x80


int
vfprintf (FILE * stream, const char *fmt, va_list ap)
{
    unsigned char c;		/* holds a char from the format string */
    unsigned char flags;
    unsigned char buf[PRINTF_BUF_SIZE];	/* size for -1 in octal, without '\0'	*/

    stream->len = 0;

    if ((stream->flags & __SWR) == 0)
	return EOF;

    for (;;) {

	for (;;) {
	    c = *fmt++;
	    if (!c) goto ret;
	    if (c == '%') {
		c = *fmt++;
		if (c != '%') break;
	    }
	    putc (c, stream);
	}

	for (flags = 0;
	     !(flags & FL_LONG);	/* 'll' will detect as error	*/
	     c = *fmt++)
	{
	    if (c && strchr(" +-.0123456789h", c))
		continue;
	    if (c == '#') {
		flags |= FL_ALT;
		continue;
	    }
	    if (c == 'l') {
		flags |= FL_LONG;
		continue;
	    }
	    break;
	}

	/* Only a format character is valid.	*/

	if (c && strchr("EFGefg", c)) {
		(void) PRINTF_FLOAT_ARG(ap);
	    putc ('?', stream);
	    continue;
	}

	{
	    const char * pnt;

	    switch (c) {

	      case 'c':
		putc (va_arg (ap, int), stream);
		continue;

	      case 'S':
		/* FALLTHROUGH */
	      case 's':
		pnt = va_arg (ap, char *);
	        while ( (c = *pnt++) != 0)
		    putc (c, stream);
		continue;
	    }
	}

	if (c == 'd' || c == 'i') {
	    ultoa_signed_t x = arg_to_signed(flags);

	    flags &= ~FL_ALT;
	    if (x < 0) {
		x = -x;
		/* `putc ('-', stream)' will considarably inlarge stack size.
		   So flag is used.	*/
		flags |= FL_NEGATIVE;
	    }
	    c = __ultoa_invert (x, (char *)buf, 10) - (char *)buf;

	} else {
	    int base;

	    switch (c) {
	      case 'u':
		flags &= ~FL_ALT;
	        base = 10;
		goto ultoa;
	      case 'o':
	        base = 8;
		goto ultoa;
	      case 'p':
	        flags |= FL_ALT;
		/* no break */
	      case 'x':
		flags |= (FL_ALTHEX | FL_ALTLWR);
	        base = 16;
		goto ultoa;
	      case 'X':
		flags |= FL_ALTHEX;
	        base = 16 | XTOA_UPPER;
	      ultoa:
		c = __ultoa_invert (arg_to_unsigned(flags), (char *)buf, base) - (char *)buf;
		break;

	      default:
	        goto ret;
	    }
	}

	/* Integer number output.	*/
	if (flags & FL_NEGATIVE)
	    putc ('-', stream);
	if ((flags & FL_ALT) && (buf[c-1] != '0')) {
	    putc ('0', stream);
	    if (flags & FL_ALTHEX)
#if  FL_ALTLWR != 'x' - 'X'
# error
#endif
		putc ('X' + (flags & FL_ALTLWR), stream);
	}
	do {
	    putc (buf[--c], stream);
	} while (c);

    } /* for (;;) */

  ret:
    return stream->len;
}

/* --------------------------------------------------------------------	*/
#else	/* i.e. PRINTF_LEVEL > PRINTF_MIN */

/* Order is relevant here and matches order in format string */

#define FL_ZFILL	0x0001
#define FL_PLUS		0x0002
#define FL_SPACE	0x0004
#define FL_LPAD		0x0008
#define FL_ALT		0x0010

#define FL_WIDTH	0x0020
#define FL_PREC		0x0040

#define FL_LONG		0x0080
#define FL_SHORT	0x0100
#define FL_REPD_TYPE	0x0200

#define FL_NEGATIVE	0x0400

#define FL_ALTUPP	0x0800
#define FL_ALTHEX	0x1000

#define	FL_FLTUPP	0x2000
#define FL_FLTEXP	0x4000
#define	FL_FLTFIX	0x8000

int vfprintf (FILE * stream, const char *fmt, va_list ap)
{
    unsigned char c;		/* holds a char from the format string */
    uint16_t flags;
    int width;
    int prec;
    union {
	unsigned char __buf[PRINTF_BUF_SIZE];	/* size for -1 in octal, without '\0'	*/
#if PRINTF_LEVEL >= PRINTF_FLT
	struct dtoa __dtoa;
#endif
    } u;
    const char * pnt;
    size_t size;
    unsigned char len;

#define buf	(u.__buf)
#define _dtoa	(u.__dtoa)

    stream->len = 0;

    if ((stream->flags & __SWR) == 0)
	return EOF;

    for (;;) {

	for (;;) {
	    c = *fmt++;
	    if (!c) goto ret;
	    if (c == '%') {
		c = *fmt++;
		if (c != '%') break;
	    }
	    putc (c, stream);
	}

	flags = 0;
	width = 0;
	prec = 0;
	
	do {
	    if (flags < FL_WIDTH) {
		switch (c) {
		  case '0':
		    flags |= FL_ZFILL;
		    continue;
		  case '+':
		    flags |= FL_PLUS;
		    /* FALLTHROUGH */
		  case ' ':
		    flags |= FL_SPACE;
		    continue;
		  case '-':
		    flags |= FL_LPAD;
		    continue;
		  case '#':
		    flags |= FL_ALT;
		    continue;
		}
	    }

	    if (flags < FL_LONG) {
		if (c >= '0' && c <= '9') {
		    c -= '0';
		    if (flags & FL_PREC) {
			prec = 10*prec + c;
			continue;
		    }
		    width = 10*width + c;
		    flags |= FL_WIDTH;
		    continue;
		}
		if (c == '*') {
		    if (flags & FL_PREC) {
			prec = va_arg(ap, int);
			if (prec < 0)
			    prec = 0;
		    } else {
			width = va_arg(ap, int);
			flags |= FL_WIDTH;
			if (width < 0) {
			    width = -width;
			    flags |= FL_LPAD;
			}
		    }
		    continue;
		}
		if (c == '.') {
		    if (flags & FL_PREC)
			goto ret;
		    flags |= FL_PREC;
		    continue;
		}
	    }

	    if (c == 'l') {
		if (flags & FL_LONG)
		    flags |= FL_REPD_TYPE;
		flags |= FL_LONG;
		flags &= ~FL_SHORT;
		continue;
	    }

	    if (c == 'h') {
		if (flags & FL_SHORT)
		    flags |= FL_REPD_TYPE;
		flags |= FL_SHORT;
		flags &= ~FL_LONG;
		continue;
	    }

	    break;
	} while ( (c = *fmt++) != 0);

	/* Only a format character is valid.	*/

#if	'F' != 'E'+1  ||  'G' != 'F'+1  ||  'f' != 'e'+1  ||  'g' != 'f'+1
# error
#endif

#if PRINTF_LEVEL >= PRINTF_FLT
	if (c >= 'E' && c <= 'G') {
	    flags |= FL_FLTUPP;
	    c += 'e' - 'E';
	    goto flt_oper;

	} else if (c >= 'e' && c <= 'g') {
	    int exp;			/* exponent of master decimal digit	*/
	    int n;
	    uint8_t sign;		/* sign character (or 0)	*/
	    uint8_t ndigs;		/* number of digits to convert */
	    uint8_t ndigs_exp;		/* number of digis in exponent */

	    flags &= ~FL_FLTUPP;

	  flt_oper:
	    ndigs = 0;
	    if (!(flags & FL_PREC))
		prec = 6;
	    flags &= ~(FL_FLTEXP | FL_FLTFIX);

	    uint8_t ndecimal;	/* digits after decimal (for 'f' format), 0 if no limit */

	    if (c == 'e') {
		ndigs = prec + 1;
		ndecimal = 0;
		flags |= FL_FLTEXP;
	    } else if (c == 'f') {
		ndigs = DTOA_MAX_DIG;
		ndecimal = prec + 1;
		flags |= FL_FLTFIX;
	    } else {
		ndigs = prec;
		if (ndigs < 1) ndigs = 1;
		ndecimal = 0;
	    }

	    if (ndigs > DTOA_MAX_DIG)
		ndigs = DTOA_MAX_DIG;

	    ndigs = __dtoa_engine (PRINTF_FLOAT_ARG(ap), &_dtoa, ndigs, ndecimal);
	    exp = _dtoa.exp;
	    ndigs_exp = 2;
#ifndef PICOLIBC_FLOAT_PRINTF_SCANF
	    if (exp < -99 || 99 < exp)
		    ndigs_exp = 3;
#endif

	    sign = 0;
	    if ((_dtoa.flags & DTOA_MINUS) && !(_dtoa.flags & DTOA_NAN))
		sign = '-';
	    else if (flags & FL_PLUS)
		sign = '+';
	    else if (flags & FL_SPACE)
		sign = ' ';

	    if (_dtoa.flags & (DTOA_NAN | DTOA_INF))
	    {
		const char *p;
		ndigs = sign ? 4 : 3;
		if (width > ndigs) {
		    width -= ndigs;
		    if (!(flags & FL_LPAD)) {
			do {
			    putc (' ', stream);
			} while (--width);
		    }
		} else {
		    width = 0;
		}
		if (sign)
		    putc (sign, stream);
		p = "inf";
		if (_dtoa.flags & DTOA_NAN)
		    p = "nan";
# if ('I'-'i' != 'N'-'n') || ('I'-'i' != 'F'-'f') || ('I'-'i' != 'A'-'a')
#  error
# endif
		while ( (ndigs = *p) != 0) {
		    if (flags & FL_FLTUPP)
			ndigs += 'I' - 'i';
		    putc (ndigs, stream);
		    p++;
		}
		goto tail;
	    }

	    if (!(flags & (FL_FLTEXP|FL_FLTFIX))) {

		/* 'g(G)' format */

		prec = ndigs;

		/* Remove trailing zeros unless '#' */
		if (!(flags & FL_ALT))
		    while (ndigs > 0 && _dtoa.digits[ndigs-1] == '0')
			ndigs--;

		if (-4 <= exp && exp < prec)
		{
		    flags |= FL_FLTFIX;

		    if (exp < 0 || ndigs > exp)
			prec = ndigs - (exp + 1);
		    else
			prec = 0;
		} else {

		    /* Limit displayed precision to available precision */
		    prec = ndigs - 1;
		}
	    }

	    /* Conversion result length, width := free space length	*/
	    if (flags & FL_FLTFIX)
		n = (exp>0 ? exp+1 : 1);
	    else
		n = 3 + ndigs_exp;		/* 1e+00 */
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
		    putc (' ', stream);
		    width--;
		}
	    }
	    if (sign)
		putc (sign, stream);

	    if (!(flags & FL_LPAD)) {
		while (width) {
		    putc ('0', stream);
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
			putc ('.', stream);

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
		    putc (out, stream);
		} while (1);
		if (n == exp
		    && (_dtoa.digits[0] > '5'
		        || (_dtoa.digits[0] == '5' && !(_dtoa.flags & DTOA_CARRY))) )
		{
		    out = '1';
		}
		putc (out, stream);
		if ((flags & FL_ALT) && n == -1)
			putc('.', stream);
	    } else {				/* 'e(E)' format	*/

		/* mantissa	*/
		if (_dtoa.digits[0] != '1')
		    _dtoa.flags &= ~DTOA_CARRY;
		putc (_dtoa.digits[0], stream);
		if (prec > 0) {
		    putc ('.', stream);
		    uint8_t pos = 1;
		    for (pos = 1; pos < 1 + prec; pos++)
			putc (pos < ndigs ? _dtoa.digits[pos] : '0', stream);
		} else if (flags & FL_ALT)
		    putc ('.', stream);

		/* exponent	*/
		putc (flags & FL_FLTUPP ? 'E' : 'e', stream);
		ndigs = '+';
		if (exp < 0 || (exp == 0 && (_dtoa.flags & DTOA_CARRY) != 0)) {
		    exp = -exp;
		    ndigs = '-';
		}
		putc (ndigs, stream);
#ifndef PICOLIBC_FLOAT_PRINTF_SCANF
		if (ndigs_exp > 2) {
			putc(exp / 100 + '0', stream);
			exp %= 100;
		}
#endif
		putc(exp / 10 + '0', stream);
		exp %= 10;
		putc ('0' + exp, stream);
	    }

	    goto tail;
	}

#else		/* to: PRINTF_LEVEL >= PRINTF_FLT */
	if ((c >= 'E' && c <= 'G') || (c >= 'e' && c <= 'g')) {
	    (void) PRINTF_FLOAT_ARG(ap);
	    pnt = "*float*";
	    size = sizeof ("*float*") - 1;
	    goto str_lpad;
	}
#endif

	switch (c) {

	case 'c':
	    buf[0] = va_arg (ap, int);
	    pnt = (char *)buf;
	    size = 1;
	    goto str_lpad;

	case 's':
	case 'S':
	    pnt = va_arg (ap, char *);
	    if (!pnt)
		pnt = "(null)";
	    size = strnlen (pnt, (flags & FL_PREC) ? prec : ~0);

	str_lpad:
	    if (!(flags & FL_LPAD)) {
		while (size < width) {
		    putc (' ', stream);
		    width--;
		}
	    }
	    while (size) {
		putc (*pnt++, stream);
		if (width) width -= 1;
		size -= 1;
	    }
	    goto tail;
	}

	if (c == 'd' || c == 'i') {
	    ultoa_signed_t x = arg_to_signed(flags);

	    flags &= ~(FL_NEGATIVE | FL_ALT);
	    if (x < 0) {
		x = -x;
		flags |= FL_NEGATIVE;
	    }

	    if ((flags & FL_PREC) && prec == 0 && x == 0)
		c = 0;
	    else
		c = __ultoa_invert (x, (char *)buf, 10) - (char *)buf;
	} else {
	    int base;
	    ultoa_unsigned_t x = arg_to_unsigned(flags);

	    flags &= ~(FL_PLUS | FL_SPACE);

	    switch (c) {
	      case 'u':
		flags &= ~FL_ALT;
		base = 10;
		break;
	      case 'o':
	        base = 8;
		break;
	      case 'p':
	        flags |= FL_ALT;
		/* no break */
	      case 'x':
		if (flags & FL_ALT)
		    flags |= FL_ALTHEX;
	        base = 16;
		break;
	      case 'X':
		if (flags & FL_ALT)
		    flags |= (FL_ALTHEX | FL_ALTUPP);
	        base = 16 | XTOA_UPPER;
		break;
	      default:
		putc('%', stream);
		putc(c, stream);
		continue;
	    }
	    if ((flags & FL_PREC) && prec == 0 && x == 0)
		c = 0;
	    else
		c = __ultoa_invert (x, (char *)buf, base) - (char *)buf;
	    flags &= ~FL_NEGATIVE;
	}

	len = c;

	if (flags & FL_PREC) {
	    flags &= ~FL_ZFILL;
	    if (len < prec) {
		len = prec;
		if ((flags & FL_ALT) && !(flags & FL_ALTHEX))
		    flags &= ~FL_ALT;
	    }
	}
	if (flags & FL_ALT) {
	    if (buf[c-1] == '0') {
		flags &= ~(FL_ALT | FL_ALTHEX | FL_ALTUPP);
	    } else {
		len += 1;
		if (flags & FL_ALTHEX)
		    len += 1;
	    }
	} else if (flags & (FL_NEGATIVE | FL_PLUS | FL_SPACE)) {
	    len += 1;
	}

	if (!(flags & FL_LPAD)) {
	    if (flags & FL_ZFILL) {
		prec = c;
		if (len < width) {
		    prec += width - len;
		    len = width;
		}
	    }
	    while (len < width) {
		putc (' ', stream);
		len++;
	    }
	}

	width =  (len < width) ? width - len : 0;

	if (flags & FL_ALT) {
	    putc ('0', stream);
	    if (flags & FL_ALTHEX)
		putc (flags & FL_ALTUPP ? 'X' : 'x', stream);
	} else if (flags & (FL_NEGATIVE | FL_PLUS | FL_SPACE)) {
	    unsigned char z = ' ';
	    if (flags & FL_PLUS) z = '+';
	    if (flags & FL_NEGATIVE) z = '-';
	    putc (z, stream);
	}

	while (prec > c) {
	    putc ('0', stream);
	    prec--;
	}

	while (c)
	    putc (buf[--c], stream);

      tail:
	/* Tail is possible.	*/
	while (width) {
	    putc (' ', stream);
	    width--;
	}
    } /* for (;;) */

  ret:
    return stream->len;
}

#endif	/* PRINTF_LEVEL > PRINTF_MIN */
