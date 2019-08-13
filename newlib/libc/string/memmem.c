/* Optimized memmem function.
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
	<<memmem>>---find memory segment

INDEX
	memmem

SYNOPSIS
	#include <string.h>
	void *memmem(const void *<[s1]>, size_t <[l1]>, const void *<[s2]>,
		     size_t <[l2]>);

DESCRIPTION

	Locates the first occurrence in the memory region pointed to
	by <[s1]> with length <[l1]> of the sequence of bytes pointed
	to by <[s2]> of length <[l2]>.  If you already know the
	lengths of your haystack and needle, <<memmem>> is much faster
	than <<strstr>>.

RETURNS
	Returns a pointer to the located segment, or a null pointer if
	<[s2]> is not found. If <[l2]> is 0, <[s1]> is returned.

PORTABILITY
<<memmem>> is a newlib extension.

<<memmem>> requires no supporting OS subroutines.

QUICKREF
	memmem pure
*/

#include <string.h>
#include <stdint.h>

#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)

/* Small and efficient memmem implementation (quadratic worst-case).  */
void *
memmem (const void *haystack, size_t hs_len, const void *needle, size_t ne_len)
{
  const char *hs = haystack;
  const char *ne = needle;

  if (ne_len == 0)
    return (void *)hs;
  int i;
  int c = ne[0];
  const char *end = hs + hs_len - ne_len;

  for ( ; hs <= end; hs++)
  {
    if (hs[0] != c)
      continue;
    for (i = ne_len - 1; i != 0; i--)
      if (hs[i] != ne[i])
	break;
    if (i == 0)
      return (void *)hs;
  }

  return NULL;
}

#else

# define RETURN_TYPE void *
# define AVAILABLE(h, h_l, j, n_l) ((j) <= (h_l) - (n_l))
# include "str-two-way.h"

#define hash2(p) (((size_t)(p)[0] - ((size_t)(p)[-1] << 3)) % sizeof (shift))

/* Fast memmem algorithm with guaranteed linear-time performance.
   Small needles up to size 2 use a dedicated linear search.  Longer needles
   up to size 256 use a novel modified Horspool algorithm.  It hashes pairs
   of characters to quickly skip past mismatches.  The main search loop only
   exits if the last 2 characters match, avoiding unnecessary calls to memcmp
   and allowing for a larger skip if there is no match.  A self-adapting
   filtering check is used to quickly detect mismatches in long needles.
   By limiting the needle length to 256, the shift table can be reduced to 8
   bits per entry, lowering preprocessing overhead and minimizing cache effects.
   The limit also implies worst-case performance is linear.
   Needles larger than 256 characters use the linear-time Two-Way algorithm.  */
void *
memmem (const void *haystack, size_t hs_len, const void *needle, size_t ne_len)
{
  const unsigned char *hs = haystack;
  const unsigned char *ne = needle;

  if (ne_len == 0)
    return (void *) hs;
  if (ne_len == 1)
    return (void *) memchr (hs, ne[0], hs_len);

  /* Ensure haystack length is >= needle length.  */
  if (hs_len < ne_len)
    return NULL;

  const unsigned char *end = hs + hs_len - ne_len;

  if (ne_len == 2)
    {
      uint32_t nw = ne[0] << 16 | ne[1], hw = hs[0] << 16 | hs[1];
      for (hs++; hs <= end && hw != nw; )
	hw = hw << 16 | *++hs;
      return hw == nw ? (void *)(hs - 1) : NULL;
    }

  /* Use Two-Way algorithm for very long needles.  */
  if (__builtin_expect (ne_len > 256, 0))
    return two_way_long_needle (hs, hs_len, ne, ne_len);

  uint8_t shift[256];
  size_t tmp, shift1;
  size_t m1 = ne_len - 1;
  size_t offset = 0;
  int i;

  /* Initialize bad character shift hash table.  */
  memset (shift, 0, sizeof (shift));
  for (i = 1; i < m1; i++)
    shift[hash2 (ne + i)] = i;
  shift1 = m1 - shift[hash2 (ne + m1)];
  shift[hash2 (ne + m1)] = m1;

  for ( ; hs <= end; )
    {
      /* Skip past character pairs not in the needle.  */
      do
	{
	  hs += m1;
	  tmp = shift[hash2 (hs)];
	}
      while (hs <= end && tmp == 0);

      /* If the match is not at the end of the needle, shift to the end
	 and continue until we match the last 2 characters.  */
      hs -= tmp;
      if (tmp < m1)
	continue;

      /* The last 2 characters match.  If the needle is long, check a
	 fixed number of characters first to quickly filter out mismatches.  */
      if (m1 <= 15 || memcmp (hs + offset, ne + offset, sizeof (long)) == 0)
	{
	  if (memcmp (hs, ne, m1) == 0)
	    return (void *) hs;

	  /* Adjust filter offset when it doesn't find the mismatch.  */
	  offset = (offset >= sizeof (long) ? offset : m1) - sizeof (long);
	}

      /* Skip based on matching the last 2 characters.  */
      hs += shift1;
    }
  return NULL;
}
#endif /* Compilation for speed.  */
