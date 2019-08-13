/* Optimized strstr function.
   Copyright (c) 2018 Arm Ltd.  All rights reserved.

   SPDX-License-Identifier: BSD-3-Clause

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. The name of the company may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

   THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

/*
FUNCTION
	<<strstr>>---find string segment

INDEX
	strstr

SYNOPSIS
	#include <string.h>
	char *strstr(const char *<[s1]>, const char *<[s2]>);

DESCRIPTION
	Locates the first occurrence in the string pointed to by <[s1]> of
	the sequence of characters in the string pointed to by <[s2]>
	(excluding the terminating null character).

RETURNS
	Returns a pointer to the located string segment, or a null
	pointer if the string <[s2]> is not found. If <[s2]> points to
	a string with zero length, <[s1]> is returned.

PORTABILITY
<<strstr>> is ANSI C.

<<strstr>> requires no supporting OS subroutines.

QUICKREF
	strstr ansi pure
*/

#include <string.h>
#include <limits.h>

#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__) \
    || CHAR_BIT > 8

/* Small and efficient strstr implementation.  */
char *
strstr (const char *hs, const char *ne)
{
  size_t i;
  int c = ne[0];

  if (c == 0)
    return (char*)hs;

  for ( ; hs[0] != '\0'; hs++)
    {
      if (hs[0] != c)
	continue;
      for (i = 1; ne[i] != 0; i++)
	if (hs[i] != ne[i])
	  break;
      if (ne[i] == '\0')
	return (char*)hs;
    }

  return NULL;
}

#else /* compilation for speed */

# define RETURN_TYPE char *
# define AVAILABLE(h, h_l, j, n_l) (((j) <= (h_l) - (n_l)) \
   || ((h_l) += strnlen ((h) + (h_l), (n_l) | 2048), ((j) <= (h_l) - (n_l))))

# include "str-two-way.h"

/* Number of bits used to index shift table.  */
#define SHIFT_TABLE_BITS 6

static inline char *
strstr2 (const unsigned char *hs, const unsigned char *ne)
{
  uint32_t h1 = (ne[0] << 16) | ne[1];
  uint32_t h2 = 0;
  int c;
  for (c = hs[0]; h1 != h2 && c != 0; c = *++hs)
      h2 = (h2 << 16) | c;
  return h1 == h2 ? (char *)hs - 2 : NULL;
}

static inline char *
strstr3 (const unsigned char *hs, const unsigned char *ne)
{
  uint32_t h1 = (ne[0] << 24) | (ne[1] << 16) | (ne[2] << 8);
  uint32_t h2 = 0;
  int c;
  for (c = hs[0]; h1 != h2 && c != 0; c = *++hs)
      h2 = (h2 | c) << 8;
  return h1 == h2 ? (char *)hs - 3 : NULL;
}

static inline char *
strstr4 (const unsigned char *hs, const unsigned char *ne)
{
  uint32_t h1 = (ne[0] << 24) | (ne[1] << 16) | (ne[2] << 8) | ne[3];
  uint32_t h2 = 0;
  int c;
  for (c = hs[0]; c != 0 && h1 != h2; c = *++hs)
    h2 = (h2 << 8) | c;
  return h1 == h2 ? (char *)hs - 4 : NULL;
}

/* Extremely fast strstr algorithm with guaranteed linear-time performance.
   Small needles up to size 4 use a dedicated linear search.  Longer needles
   up to size 254 use Sunday's Quick-Search algorithm.  Due to its simplicity
   it has the best average performance of string matching algorithms on almost
   all inputs.  It uses a bad-character shift table to skip past mismatches.
   By limiting the needle length to 254, the shift table can be reduced to 8
   bits per entry, lowering preprocessing overhead and minimizing cache effects.
   The limit also implies the worst-case performance is linear.
   Even larger needles are processed by the linear-time Two-Way algorithm.
*/
char *
strstr (const char *haystack, const char *needle)
{
  const unsigned char *hs = (const unsigned char *) haystack;
  const unsigned char *ne = (const unsigned char *) needle;
  int i;

  /* Handle short needle special cases first.  */
  if (ne[0] == '\0')
    return (char *) hs;
  if (ne[1] == '\0')
    return (char*)strchr (hs, ne[0]);
  if (ne[2] == '\0')
    return strstr2 (hs, ne);
  if (ne[3] == '\0')
    return strstr3 (hs, ne);
  if (ne[4] == '\0')
    return strstr4 (hs, ne);

  size_t ne_len = strlen (ne);
  size_t hs_len = strnlen (hs, ne_len | 512);

  /* Ensure haystack length is >= needle length.  */
  if (hs_len < ne_len)
    return NULL;

  /* Use the Quick-Search algorithm for needle lengths less than 255.  */
  if (__builtin_expect (ne_len < 255, 1))
    {
      uint8_t shift[1 << SHIFT_TABLE_BITS];
      const unsigned char *end = hs + hs_len - ne_len;

      /* Initialize bad character shift hash table.  */
      memset (shift, ne_len + 1, sizeof (shift));
      for (i = 0; i < ne_len; i++)
	shift[ne[i] % sizeof (shift)] = ne_len - i;

      do
	{
	  hs--;

	  /* Search by skipping past bad characters.  */
	  size_t tmp = shift[hs[ne_len] % sizeof (shift)];
	  for (hs += tmp; hs <= end; hs += tmp)
	    {
	      tmp = shift[hs[ne_len] % sizeof (shift)];
	      if (memcmp (hs, ne, ne_len) == 0)
		return (char*) hs;
	    }
	  if (end[ne_len] == 0)
	    return NULL;
	  end += strnlen (end + ne_len, 2048);
	}
      while (hs <= end);

      return NULL;
    }

  /* Use Two-Way algorithm for very long needles.  */
  return two_way_long_needle (hs, hs_len, ne, ne_len);
}
#endif /* compilation for speed */
