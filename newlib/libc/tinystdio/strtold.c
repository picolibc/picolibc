/* Copyright (c) 2002-2005  Michael Stumpf  <mistumpf@de.pepperl-fuchs.com>
   Copyright (c) 2006,2008  Dmitry Xmelkov

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
   POSSIBILITY OF SUCH DAMAGE. */

/* $Id: strtod.c 2191 2010-11-05 13:45:57Z arcanum $ */


#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>		/* INFINITY, NAN		*/
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

#define NPOW_10	13

static const long double pwr_p10 [NPOW_10] = {
    1e+1L, 1e+2L, 1e+4L, 1e+8L, 1e+16L, 1e+32L, 1e+64L, 1e+128L, 1e+256L, 1e+512L, 1e+1024L, 1e+2048L, 1e+4096L
};

static const long double pwr_m10 [NPOW_10] = {
    1e-1L, 1e-2L, 1e-4L, 1e-8L, 1e-16L, 1e-32L, 1e-64L, 1e-128L, 1e-256L, 1e-512L, 1e-1024L, 1e-2048L, 1e-4096L
};

/**  The strtold() function converts the initial portion of the string pointed
     to by \a nptr to long double representation.

     The expected form of the string is an optional plus ( \c '+' ) or minus
     sign ( \c '-' ) followed by a sequence of digits optionally containing
     a decimal-point character, optionally followed by an exponent.  An
     exponent consists of an \c 'E' or \c 'e', followed by an optional plus
     or minus sign, followed by a sequence of digits.

     Leading white-space characters in the string are skipped.

     The strtold() function returns the converted value, if any.

     If \a endptr is not \c NULL, a pointer to the character after the last
     character used in the conversion is stored in the location referenced by
     \a endptr.

     If no conversion is performed, zero is returned and the value of
     \a nptr is stored in the location referenced by \a endptr.

     If the correct value would cause overflow, plus or minus \c INFINITY is
     returned (according to the sign of the value), and \c ERANGE is stored
     in \c errno.  If the correct value would cause underflow, zero is
     returned and \c ERANGE is stored in \c errno.
 */

#ifdef __SIZEOF_INT128__
typedef __uint128_t _u128;
#define _u128_plus_64(a,b) ((a) + (b))
#define _u128_plus(a,b) ((a) + (b))
#define _u128_times_10(a) ((a) * 10)
#define _u128_to_ld(a) ((long double) (a))
#define _u128_oflow(a)	((a) >= (((((_u128) 0xffffffffffffffffULL) << 64) | 0xffffffffffffffffULL) - 9 / 10))
#define _u128_zero	(_u128) 0
#else
typedef struct {
    uint64_t	hi, lo;
} _u128;
#define _u128_zero	(_u128) { 0, 0 }

static _u128
_u128_plus_64(_u128 a, uint64_t b)
{
    _u128 v;

    v.lo = a.lo + b;
    v.hi = a.hi;
    if (v.lo < a.lo)
	v.hi++;
    return v;
}

static _u128
_u128_plus(_u128 a, _u128 b)
{
    _u128 v;

    v.lo = a.lo + b.lo;
    v.hi = a.hi + b.hi;
    if (v.lo < a.lo)
	v.hi++;
    return v;
}

static _u128
_u128_lshift(_u128 a, int amt)
{
    _u128	v;

    v.lo = a.lo << amt;
    v.hi = (a.lo >> (64 - amt)) | (a.hi << amt);
    return v;
}

static _u128
_u128_times_10(_u128 a)
{
    return _u128_plus(_u128_lshift(a, 3), _u128_lshift(a, 1));
}

static long double
_u128_to_ld(_u128 a)
{
    return (long double) a.hi * ((long double) (1LL << 32) * (long double) (1LL << 32)) + (long double) a.lo;
}

static bool
_u128_oflow(_u128 a)
{
    return a.hi >= (0xffffffffffffffffULL - 9) / 10;
}
#endif
#include "stdio_private.h"

long double
strtold (const char * nptr, char ** endptr)
{
    _u128 u128;
    long double flt;
    unsigned char c;
    int exp;

    unsigned char flag;
#define FL_MINUS    0x01	/* number is negative	*/
#define FL_ANY	    0x02	/* any digit was readed	*/
#define FL_OVFL	    0x04	/* overflow was		*/
#define FL_DOT	    0x08	/* decimal '.' was	*/
#define FL_MEXP	    0x10	/* exponent 'e' is neg.	*/

    if (endptr)
	*endptr = (char *)nptr;

    do {
	c = *nptr++;
    } while (isspace (c));

    flag = 0;
    if (c == '-') {
	flag = FL_MINUS;
	c = *nptr++;
    } else if (c == '+') {
	c = *nptr++;
    }

    if (__matchcaseprefix(nptr - 1, __match_inf)) {
	nptr += 2;
	if (__matchcaseprefix(nptr, __match_inity))
	    nptr += 5;
	if (endptr)
	    *endptr = (char *)nptr;
	return flag & FL_MINUS ? -(long double)INFINITY : +(long double)INFINITY;
    }

    /* NAN() construction is not realised.
       Length would be 3 characters only.	*/
    if (__matchcaseprefix(nptr - 1, __match_nan)) {
	if (endptr)
	    *endptr = (char *)nptr + 2;
	return (long double) NAN;
    }

    u128 = _u128_zero;
    exp = 0;
    while (1) {

	c -= '0';

	if (c <= 9) {
	    flag |= FL_ANY;
	    if (flag & FL_OVFL) {
		if (!(flag & FL_DOT))
		    exp += 1;
	    } else {
		if (flag & FL_DOT)
		    exp -= 1;
		u128 = _u128_plus_64(_u128_times_10(u128), c);
		if (_u128_oflow(u128))
		    flag |= FL_OVFL;
	    }

	} else if (c == (('.'-'0') & 0xff)  &&  !(flag & FL_DOT)) {
	    flag |= FL_DOT;
	} else {
	    break;
	}
	c = *nptr++;
    }

    if (c == (('e'-'0') & 0xff) || c == (('E'-'0') & 0xff))
    {
	int i;
	c = *nptr++;
	i = 2;
	if (c == '-') {
	    flag |= FL_MEXP;
	    c = *nptr++;
	} else if (c == '+') {
	    c = *nptr++;
	} else {
	    i = 1;
	}
	c -= '0';
	if (c > 9) {
	    nptr -= i;
	} else {
	    i = 0;
	    do {
		i = i * 10 + c;
		c = *nptr++ - '0';
	    } while (c <= 9);
	    if (flag & FL_MEXP)
		i = -i;
	    exp += i;
	}
    }

    if ((flag & FL_ANY) && endptr)
	*endptr = (char *)nptr - 1;

    flt = _u128_to_ld(u128);
    if ((flag & FL_MINUS) && (flag & FL_ANY))
	flt = -flt;

    if (flt != 0) {
	int pwr;
	const long double *pptr;
	if (exp < 0) {
	    pptr = (pwr_m10 + NPOW_10 - 1);
	    exp = -exp;
	} else {
	    pptr = (pwr_p10 + NPOW_10 - 1);
	}
	for (pwr = 1 << (NPOW_10 - 1); pwr; pwr >>= 1) {
	    for (; exp >= pwr; exp -= pwr) {
		flt *= *pptr;
	    }
	    pptr--;
	}
	if (flt == (long double) INFINITY || flt == 0.0L)
	    errno = ERANGE;
    }

    return flt;
}
