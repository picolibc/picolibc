/*
FUNCTION
	<<strcasestr>>---case-insensitive character string search

INDEX
	strcasestr

SYNOPSIS
	#include <string.h>
	char *strcasestr(const char *<[s]>, const char *<[find]>);

DESCRIPTION
	<<strcasestr>> searchs the string <[s]> for
	the first occurrence of the sequence <[find]>.  <<strcasestr>>
	is identical to <<strstr>> except the search is
	case-insensitive.

RETURNS

	A pointer to the first case-insensitive occurrence of the sequence
	<[find]> or <<NULL>> if no match was found.

PORTABILITY
<<strcasestr>> is in the Berkeley Software Distribution.

<<strcasestr>> requires no supporting OS subroutines. It uses
tolower() from elsewhere in this library.

QUICKREF
	strcasestr
*/

/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * The quadratic code is derived from software contributed to Berkeley by
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
/* Linear algorithm Copyright (C) 2008 Eric Blake
 * Permission to use, copy, modify, and distribute the linear portion of
 * software is freely granted, provided that this notice is preserved.
 */

#include <sys/cdefs.h>

#include <ctype.h>
#include <string.h>
#include <strings.h>

#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
# define RETURN_TYPE char *
# define AVAILABLE(h, h_l, j, n_l)			\
  (!memchr ((h) + (h_l), '\0', (j) + (n_l) - (h_l))	\
   && ((h_l) = (j) + (n_l)))
# define CANON_ELEMENT(c) tolower (c)
#if __GNUC_PREREQ (4, 2)
/* strncasecmp uses signed char, CMP_FUNC is expected to use unsigned char. */
#pragma GCC diagnostic ignored "-Wpointer-sign"
#endif
# define CMP_FUNC strncasecmp
# include "str-two-way.h"
#endif

/*
 * Find the first occurrence of find in s, ignore case.
 */
char *
strcasestr (const char *s,
	const char *find)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)

  /* Less code size, but quadratic performance in the worst case.  */
	char c, sc;
	size_t len;

	if ((c = *find++) != 0) {
		c = tolower((unsigned char)c);
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while ((char)tolower((unsigned char)sc) != c);
		} while (strncasecmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);

#else /* compilation for speed */

  /* Larger code size, but guaranteed linear performance.  */
  const char *haystack = s;
  const char *needle = find;
  size_t needle_len; /* Length of NEEDLE.  */
  size_t haystack_len; /* Known minimum length of HAYSTACK.  */
  int ok = 1; /* True if NEEDLE is prefix of HAYSTACK.  */

  /* Determine length of NEEDLE, and in the process, make sure
     HAYSTACK is at least as long (no point processing all of a long
     NEEDLE if HAYSTACK is too short).  */
  while (*haystack && *needle)
    ok &= (tolower ((unsigned char) *haystack++)
	   == tolower ((unsigned char) *needle++));
  if (*needle)
    return NULL;
  if (ok)
    return (char *) s;
  needle_len = needle - find;
  haystack = s + 1;
  haystack_len = needle_len - 1;

  /* Perform the search.  */
  if (needle_len < LONG_NEEDLE_THRESHOLD)
    return two_way_short_needle ((const unsigned char *) haystack,
				 haystack_len,
				 (const unsigned char *) find, needle_len);
  return two_way_long_needle ((const unsigned char *) haystack, haystack_len,
			      (const unsigned char *) find, needle_len);
#endif /* compilation for speed */
}
