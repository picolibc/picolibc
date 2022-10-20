/* Copyright (c) 2002,2004,2005 Joerg Wunsch
   Copyright (c) 2008  Dmitry Xmelkov
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

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "scanf_private.h"

#define CASE_CONVERT    ('a' - 'A')
#define TOLOW(c)        ((c) | CASE_CONVERT)

strtoi_type
strtoi(const char *__restrict nptr, char **__restrict endptr, int base)
{
    const char *s = nptr;
    strtoi_type val = 0;
    strtoi_type cutoff;
    int cutlim;
    int i;
    int neg = 0;
    int any = 0;

    if ((unsigned int) base > 36 || base == 1) {
        errno = EINVAL;
        goto bail;
    }
    do {
        i = *s++;
    } while (ISSPACE(i));

    switch (i) {
    case '-':
        neg = 1;
	FALLTHROUGH;
    case '+':
        i = *s++;
    }

    /* Leading '0' digit -- check for base indication */
    if ((unsigned char)i == '0') {
        i = *s++;

        any = 1;
        if (TOLOW(i) == 'x' && (base == 0 || base == 16)) {
            base = 16;
            any = 2;    /* mark special hex case */
            i = *s++;
	} else if (base == 0 || base == 8) {
            base = 8;
        }
    } else if (base == 0) {
        base = 10;
    }

#ifdef strtoi_signed
    strtoi_utype ucutoff = neg ? -(strtoi_utype)strtoi_min : strtoi_max;
    cutlim = ucutoff % base;
    cutoff = ucutoff / base;
#else
    cutoff = strtoi_max / base;
    cutlim = strtoi_max % base;
#endif

/* This fact is used below to parse hexidecimal digit.	*/
#if	('A' - '0') != (('a' - '0') & ~('A' ^ 'a'))
# error
#endif
    for(;;) {
	unsigned char c = i;
        if (TOLOW(c) >= 'a')
            c = TOLOW(c) + ('0' - 'a') + 10;
	c -= '0';
        if (c >= base) {
            /* hex case? Back up over 'x' */
            if (any == 2)
                s--;
            break;
	}
        if (any < 0 || val > cutoff || (val == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            val = val * base + c;
        }
        i = *s++;
    }
    if (any < 0) {
#ifdef strtoi_signed
        val = neg ? strtoi_min : strtoi_max;
#else
        val = strtoi_max;
#endif
        errno = ERANGE;
    } else if (neg)
        val = -val;
bail:
    if (endptr != NULL)
        *endptr = (char *) (any ? (s - 1) : nptr);
    return val;
}
