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
        <<memcpy>>---copy memory regions

SYNOPSIS
        #include <string.h>
        void* memcpy(void *restrict <[out]>, const void *restrict <[in]>,
                     size_t <[n]>);

DESCRIPTION
        This function copies <[n]> bytes from the memory region
        pointed to by <[in]> to the memory region pointed to by
        <[out]>.

        If the regions overlap, the behavior is undefined.

RETURNS
        <<memcpy>> returns a pointer to the first byte of the <[out]>
        region.

PORTABILITY
<<memcpy>> is ANSI C.

<<memcpy>> requires no supporting OS subroutines.

QUICKREF
        memcpy ansi pure
	*/

#include <string.h>
#include "local.h"
#include <stdint.h>

/* Distance from X to previous aligned boundary. Zero if aligned */
#define UNALIGNED(X) \
    (((uintptr_t)X & (sizeof (long) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof (long) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof (long))

/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

#undef memcpy

void *
__no_builtin
memcpy (void *__restrict dst0,
	const void *__restrict src0,
	size_t len0)
{
  char *dst = dst0;
  const char *src = src0;
#if !(defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__))
  unsigned long *aligned_dst;
  const unsigned long *aligned_src;

  /* If the size is small, or either SRC or DST is unaligned,
     then punt into the byte copy loop.  This should be rare.  */
  if (!TOO_SMALL(len0))
  {
      /*
       * Align dst and make sure
       * we won't fetch anything before src
       */
      unsigned start = LITTLEBLOCKSIZE*2 - UNALIGNED(dst);
      while(start--) {
          *dst++ = *src++;
          len0--;
      }

      aligned_dst = (unsigned long*)dst;
      int byte_shift = UNALIGNED(src);

      if (!byte_shift)
      {
          aligned_src = (unsigned long*)src;

          /* Copy 4X long words at a time if possible.  */
          while (len0 >= BIGBLOCKSIZE)
          {
              *aligned_dst++ = *aligned_src++;
              *aligned_dst++ = *aligned_src++;
              *aligned_dst++ = *aligned_src++;
              *aligned_dst++ = *aligned_src++;
              len0 -= BIGBLOCKSIZE;
          }

          /* Copy one long word at a time if possible.  */
          while (len0 >= LITTLEBLOCKSIZE)
          {
              *aligned_dst++ = *aligned_src++;
              len0 -= LITTLEBLOCKSIZE;
          }

          src = (char*)aligned_src;
      }
      else
      {
          /*
           * Fetch source words and then merge two of them for each
           * dest word:
           *
           * byte_shift      remain
           * |     |         |
           * xxxxxxxL RRRRRRRy
           *        D DDDDDDD
           *
           * We don't want to fetch past the source at all, so stop
           * when we have fewer than 'remain' bytes left
           */

          /* bytes used from the left word */
          int remain = LITTLEBLOCKSIZE - byte_shift;
          /* bit shifts for the left and right words */
          int left_shift = byte_shift << 3;
          int right_shift = remain << 3;

          aligned_src = (unsigned long*)(src - byte_shift);
          unsigned long left = *aligned_src++, right;

          while (len0 >= LITTLEBLOCKSIZE + remain) {
              right = *aligned_src++;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
              *aligned_dst++ = (left >> left_shift) | (right << right_shift);
#else
              *aligned_dst++ = (left << left_shift) | (right >> right_shift);
#endif
              left = right;
              len0 -= LITTLEBLOCKSIZE;
          }
          src = (char *)aligned_src - remain;
      }
      /* Pick up any residual with a byte copier.  */
     dst = (char*)aligned_dst;
  }
#endif

  while (len0--)
    *dst++ = *src++;

  return dst0;
}
