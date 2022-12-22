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
#include "stdio_private.h"

#if defined(_HAVE_LONG_DOUBLE) && !defined(_LDBL_EQ_DBL)

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

#define STRTOLD
#include "conv_flt.c"

long double
strtold (const char * nptr, char ** endptr)
{
    int len = 0;
    long double flt;
    unsigned char ret;

    while (ISSPACE(nptr[len]))
        len++;

    ret = conv_flt(nptr, &len, INT_MAX, &flt, FL_LONG);
    if (!ret) {
        flt = (long double) 0.0;
        len = 0;
    }
    if (endptr)
        *endptr = (char *) nptr + len;
    return flt;
}

#endif
