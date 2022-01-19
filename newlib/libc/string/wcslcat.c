/*
FUNCTION
	<<wcslcat>>---concatenate wide-character strings to specified length

SYNOPSIS
	#include <wchar.h>
	size_t wcslcat(wchar_t *<[dst]>, const wchar_t *<[src]>, size_t <[siz]>);

DESCRIPTION
	The <<wcslcat>> function appends wide characters from <[src]> to
	end of the <[dst]> wide-character string so that the resultant
	wide-character string is not more than <[siz]> wide characters
	including the terminating null wide-character code.  A terminating
	null wide character is always added unless <[siz]> is 0.  Thus,
	the maximum number of wide characters that can be appended from
	<[src]> is <[siz]> - 1. If copying takes place between objects
	that overlap, the behaviour is undefined.

RETURNS
	Wide-character string length of initial <[dst]> plus the
	wide-character string length of <[src]> (does not include
	terminating null wide-characters).  If the return value is
	greater than or equal to <[siz]>, then truncation occurred and
	not all wide characters from <[src]> were appended.

PORTABILITY
No supporting OS subroutines are required.
*/

/*      $OpenBSD: wcslcat.c,v 1.7 2019/01/25 00:19:25 millert Exp $     */

/*
 * Copyright (c) 1998, 2015 Todd C. Miller <millert@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <wchar.h>

/*
 * Appends src to string dst of size dsize (unlike strncat, dsize is the
 * full size of dst, not space left).  At most dsize-1 characters
 * will be copied.  Always NUL terminates (unless dsize <= wcslen(dst)).
 * Returns wcslen(src) + MIN(dsize, wcslen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
wcslcat (wchar_t *dst,
        const wchar_t *src,
        size_t dsize)
{
        const wchar_t *odst = dst;
        const wchar_t *osrc = src;
        size_t n = dsize;
        size_t dlen;

        /* Find the end of dst and adjust bytes left but don't go past end. */
        while (n-- != 0 && *dst != L'\0')
                dst++;
        dlen = dst - odst;
        n = dsize - dlen;

        if (n-- == 0)
                return(dlen + wcslen(src));
        while (*src != L'\0') {
                if (n != 0) {
                        *dst++ = *src;
                        n--;
                }
                src++;
        }
        *dst = L'\0';

        return(dlen + (src - osrc));    /* count does not include NUL */
}
