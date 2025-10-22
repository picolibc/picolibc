/*
Copyright (c) 1994 Cygnus Support.
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
and/or other materials related to such
distribution and use acknowledge that the software was developed
at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
endorse or promote products derived from this software without
specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
FUNCTION
	<<memchr>>---find character in memory

INDEX
	memchr

SYNOPSIS
	#include <string.h>
	void *memchr(const void *<[src]>, int <[c]>, size_t <[length]>);

DESCRIPTION
	This function searches memory starting at <<*<[src]>>> for the
	character <[c]>.  The search only ends with the first
	occurrence of <[c]>, or after <[length]> characters; in
	particular, <<NUL>> does not terminate the search.

RETURNS
	If the character <[c]> is found within <[length]> characters
	of <<*<[src]>>>, a pointer to the character is returned. If
	<[c]> is not found, then <<NULL>> is returned.

PORTABILITY
<<memchr>> is ANSI C.

<<memchr>> requires no supporting OS subroutines.

QUICKREF
	memchr ansi pure
*/

#include <string.h>
#include <limits.h>
#include "local.h"

void *
memchr (const void *src_void,
	int c,
	size_t length)
{
  const unsigned char *src = (const unsigned char *) src_void;
  unsigned char d = c;

#if !defined(__PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__) && \
    !defined(_PICOLIBC_NO_OUT_OF_BOUNDS_READS)
  unsigned long *asrc;
  unsigned long  mask;
  unsigned int i;

  while (UNALIGNED_X(src))
    {
      if (!length--)
        return NULL;
      if (*src == d)
        return (void *) src;
      src++;
    }

  if (!TOO_SMALL_LITTLE_BLOCK(length))
    {
      /* If we get this far, we know that length is large and src is
         word-aligned. */
      /* The fast code reads the source one word at a time and only
         performs the bytewise search on word-sized segments if they
         contain the search character, which is detected by XORing
         the word-sized segment with a word-sized block of the search
         character and then detecting for the presence of NUL in the
         result.  */
      asrc = (unsigned long *) src;
      mask = (unsigned long) d << 8 | (unsigned long) d;
      mask = mask << 16 | mask;
      for (i = 32; i < sizeof(mask) * 8; i <<= 1)
        mask = (mask << i) | mask;

      while (!TOO_SMALL_LITTLE_BLOCK(length))
        {
          if (DETECT_CHAR(*asrc, mask))
            break;
          length -= LITTLE_BLOCK_SIZE;
          asrc++;
        }

      /* If there are fewer than LITTLE_BLOCK_SIZE characters left,
         then we resort to the bytewise loop.  */

      src = (unsigned char *) asrc;
    }

#endif /* not __PREFER_SIZE_OVER_SPEED */

  while (length--)
    {
      if (*src == d)
        return (void *) src;
      src++;
    }

  return NULL;
}
