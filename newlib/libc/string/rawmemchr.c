/* Copyright (c) 2013 Yaakov Selkowitz <yselkowi@redhat.com> */
/*
FUNCTION
	<<rawmemchr>>---find character in memory

INDEX
	rawmemchr

SYNOPSIS
	#include <string.h>
	void *rawmemchr(const void *<[src]>, int <[c]>);

DESCRIPTION
	This function searches memory starting at <<*<[src]>>> for the
	character <[c]>.  The search only ends with the first occurrence
	of <[c]>; in particular, <<NUL>> does not terminate the search.
	No bounds checking is performed, so this function should only
	be used when it is certain that the character <[c]> will be found.

RETURNS
	A pointer to the first occurance of character <[c]>.

PORTABILITY
<<rawmemchr>> is a GNU extension.

<<rawmemchr>> requires no supporting OS subroutines.

QUICKREF
	rawmemchr
*/

#define _GNU_SOURCE
#include <string.h>
#include <limits.h>
#include "local.h"

void *
rawmemchr (const void *src_void,
	int c)
{
  const unsigned char *src = (const unsigned char *) src_void;
  unsigned char d = c;

#if !defined(__PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__) && \
    !defined(_PICOLIBC_NO_OUT_OF_BOUNDS_READS)
  unsigned long *asrc;
  unsigned long  mask;
  unsigned int i;

  while (UNALIGNED_X (src))
    {
      if (*src == d)
        return (void *) src;
      src++;
    }

  /* If we get this far, we know that src is word-aligned. */
  /* The fast code reads the source one word at a time and only
     performs the bytewise search on word-sized segments if they
     contain the search character, which is detected by XORing
     the word-sized segment with a word-sized block of the search
     character and then detecting for the presence of NUL in the
     result.  */
  asrc = (unsigned long *) src;
  mask = d << 8 | d;
  mask = mask << 16 | mask;
  for (i = 32; i < sizeof(mask) * 8; i <<= 1)
    mask = (mask << i) | mask;

  while (1)
    {
      if (DETECT_CHAR (*asrc, mask))
        break;
      asrc++;
    }

  /* We have the matching word, now we resort to a bytewise loop. */

  src = (unsigned char *) asrc;

#endif /* !__PREFER_SIZE_OVER_SPEED && !__OPTIMIZE_SIZE__ */

  while (1)
    {
      if (*src == d)
        return (void *) src;
      src++;
    }
}
