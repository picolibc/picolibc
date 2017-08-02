/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 * All rights reserved.
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
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

#include <sys/cdefs.h>
#if 0
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "from @(#)strtol.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
__FBSDID("FreeBSD: src/lib/libc/stdlib/strtoimax.c,v 1.8 2002/09/06 11:23:59 tjr Exp ");
#endif
__FBSDID("$FreeBSD: head/lib/libc/locale/wcstoimax.c 314436 2017-02-28 23:42:47Z imp $");

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <reent.h>
#include <stdint.h>
#include "../locale/setlocale.h"

/*
 * Convert a wide character string to an intmax_t integer.
 */

/*
 *Reentrant version of wcstoimax.
 */
static intmax_t
_wcstoimax_l(struct _reent *rptr, const wchar_t * __restrict nptr,
	     wchar_t ** __restrict endptr, int base, locale_t loc)
{
	const wchar_t *s = nptr;
	uintmax_t acc;
	wchar_t c;
	uintmax_t cutoff;
	int neg = 0, any, cutlim;

	/*
	 * See strtoimax for comments as to the logic used.
	 */
	do {
		c = *s++;
	} while (iswspace_l(c, loc));
	if (c == L'-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == L'+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == L'0' && (*s == L'x' || *s == L'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == L'0' ? 8 : 10;
	acc = any = 0;
	if (base < 2 || base > 36)
		goto noconv;

	cutoff = neg ? -(uintmax_t)INTMAX_MIN : INTMAX_MAX;
	cutlim = cutoff % base;
	cutoff /= base;
	for ( ; ; c = *s++) {
#ifdef notyet
		if (iswdigit_l(c, loc))
			c = digittoint_l(c, loc);
		else
#endif
		if (c >= L'0' && c <= L'9')
			c -= L'0';
		else if (c >= L'A' && c <= L'Z')
			c -= L'A' - 10;
		else if (c >= 'a' && c <= 'z')
			c -= L'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = neg ? INTMAX_MIN : INTMAX_MAX;
		rptr->_errno = ERANGE;
	} else if (!any) {
noconv:
		rptr->_errno = EINVAL;
	} else if (neg)
		acc = -acc;
	if (endptr != NULL)
		*endptr = (wchar_t *)(any ? s - 1 : nptr);
	return (acc);
}

intmax_t
_wcstoimax_r(struct _reent *rptr, const wchar_t *__restrict nptr,
	     wchar_t **__restrict endptr, int base)
{
	return _wcstoimax_l(rptr, nptr, endptr, base, __get_current_locale());
}

#ifndef _REENT_ONLY

intmax_t
wcstoimax_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
	    int base, locale_t loc)
{
	return _wcstoimax_l(_REENT, nptr, endptr, base, loc);
}

intmax_t
wcstoimax(const wchar_t* __restrict nptr, wchar_t** __restrict endptr, int base)
{
	return _wcstoimax_l(_REENT, nptr, endptr, base, __get_current_locale());
}

#endif
