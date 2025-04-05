/* Copyright (c) 2002 Jeff Johnston <jjohnstn@redhat.com> */
/*
FUNCTION
        <<memccpy>>---copy memory regions with end-token check

SYNOPSIS
        #include <string.h>
        void* memccpy(void *restrict <[out]>, const void *restrict <[in]>, 
                      int <[endchar]>, size_t <[n]>);

DESCRIPTION
        This function copies up to <[n]> bytes from the memory region
        pointed to by <[in]> to the memory region pointed to by
        <[out]>.  If a byte matching the <[endchar]> is encountered,
	the byte is copied and copying stops.

        If the regions overlap, the behavior is undefined.

RETURNS
        <<memccpy>> returns a pointer to the first byte following the
	<[endchar]> in the <[out]> region.  If no byte matching
	<[endchar]> was copied, then <<NULL>> is returned.

PORTABILITY
<<memccpy>> is a GNU extension.

<<memccpy>> requires no supporting OS subroutines.

	*/

#define _DEFAULT_SOURCE
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include "local.h"

void *
memccpy (void *__restrict dst0,
	const void *__restrict src0,
	int endchar0,
	size_t len0)
{

#if defined(__PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__) || \
    defined(_PICOLIBC_NO_OUT_OF_BOUNDS_READS)
  void *ptr = NULL;
  char *dst = (char *) dst0;
  char *src = (char *) src0;
  char endchar = endchar0 & 0xff;

  while (len0--)
    {
      if ((*dst++ = *src++) == endchar)
        {
          ptr = dst;
          break;
        }
    }

  return ptr;
#else
  void *ptr = NULL;
  unsigned char *dst = dst0;
  const unsigned char *src = src0;
  long *aligned_dst;
  const long *aligned_src;
  unsigned char endchar = endchar0 & 0xff;

  /* If the size is small, or either SRC or DST is unaligned,
     then punt into the byte copy loop.  This should be rare.  */
  if (!TOO_SMALL_LITTLE_BLOCK(len0) && !UNALIGNED_X_Y(src, dst))
    {
      unsigned int i;
      unsigned long mask;

      aligned_dst = (long*)dst;
      aligned_src = (long*)src;

      /* The fast code reads the ASCII one word at a time and only
         performs the bytewise search on word-sized segments if they
         contain the search character, which is detected by XORing
         the word-sized segment with a word-sized block of the search
         character and then detecting for the presence of NULL in the
         result.  */
      mask = endchar << 8 | endchar;
      mask = mask << 16 | mask;
      for (i = 32; i < sizeof(mask) * 8; i <<= 1)
        mask = (mask << i) | mask;

      /* Copy one long word at a time if possible.  */
      while (!TOO_SMALL_LITTLE_BLOCK(len0))
        {
          unsigned long buffer = (unsigned long)(*aligned_src);
          buffer ^=  mask;
          if (DETECT_NULL(buffer))
            break; /* endchar is found, go byte by byte from here */
          *aligned_dst++ = *aligned_src++;
          len0 -= LITTLE_BLOCK_SIZE;
        }

       /* Pick up any residual with a byte copier.  */
      dst = (unsigned char*)aligned_dst;
      src = (unsigned char*)aligned_src;
    }

  while (len0--)
    {
      if ((*dst++ = *src++) == endchar)
        {
          ptr = dst;
          break;
        }
    }

  return ptr;
#endif /* not __PREFER_SIZE_OVER_SPEED */
}
