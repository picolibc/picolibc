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

#include <_ansi.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
  (((uintptr_t)X & (sizeof (long) - 1)) | ((uintptr_t)Y & (sizeof (long) - 1)))

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof (long))

/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < LITTLEBLOCKSIZE)

/* Macros for detecting endchar */
#if LONG_MAX == 2147483647L
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#else
#if LONG_MAX == 9223372036854775807L
/* Nonzero if X (a long int) contains a NULL byte. */
#define DETECTNULL(X) (((X) - 0x0101010101010101) & ~(X) & 0x8080808080808080)
#else
#error long int is not a 32bit or 64bit type.
#endif
#endif


void *
memccpy (void *__restrict dst0,
	const void *__restrict src0,
	int endchar0,
	size_t len0)
{

#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
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
  if (!TOO_SMALL(len0) && !UNALIGNED (src, dst))
    {
      unsigned int i;
      unsigned long mask = 0;

      aligned_dst = (long*)dst;
      aligned_src = (long*)src;

      /* The fast code reads the ASCII one word at a time and only
         performs the bytewise search on word-sized segments if they
         contain the search character, which is detected by XORing
         the word-sized segment with a word-sized block of the search
         character and then detecting for the presence of NULL in the
         result.  */
      for (i = 0; i < LITTLEBLOCKSIZE; i++)
        mask = (mask << 8) + endchar;


      /* Copy one long word at a time if possible.  */
      while (len0 >= LITTLEBLOCKSIZE)
        {
          unsigned long buffer = (unsigned long)(*aligned_src);
          buffer ^=  mask;
          if (DETECTNULL (buffer))
            break; /* endchar is found, go byte by byte from here */
          *aligned_dst++ = *aligned_src++;
          len0 -= LITTLEBLOCKSIZE;
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
#endif /* not PREFER_SIZE_OVER_SPEED */
}
