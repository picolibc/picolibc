/*
FUNCTION
	<<rawmemchr>>---find character in memory

INDEX
	rawmemchr

ANSI_SYNOPSIS
	#include <string.h>
	void *rawmemchr(const void *<[src]>, int <[c]>);

TRAD_SYNOPSIS
	#include <string.h>
	void *rawmemchr(<[src]>, <[c]>)
	void *<[src]>;
	void *<[c]>;

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

#include <_ansi.h>
#include <string.h>
#include <limits.h>

/* Nonzero if X is not aligned on a "long" boundary.  */
#define UNALIGNED(X) ((long)X & (sizeof (long) - 1))

/* How many bytes are loaded each iteration of the word copy loop.  */
#define LBLOCKSIZE (sizeof (long))

/* Threshhold for punting to the bytewise iterator.  */
#define TOO_SMALL(LEN)  ((LEN) < LBLOCKSIZE)

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

#ifndef DETECTNULL
#error long int is not a 32bit or 64bit byte
#endif

/* DETECTCHAR returns nonzero if (long)X contains the byte used
   to fill (long)MASK. */
#define DETECTCHAR(X,MASK) (DETECTNULL(X ^ MASK))

_PTR
_DEFUN (rawmemchr, (src_void, c),
	_CONST _PTR src_void _AND
	int c)
{
  _CONST unsigned char *src = (_CONST unsigned char *) src_void;
  unsigned char d = c;

#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
  unsigned long *asrc;
  unsigned long  mask;
  int i;

  while (UNALIGNED (src))
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
  for (i = 32; i < LBLOCKSIZE * 8; i <<= 1)
    mask = (mask << i) | mask;

  while (1)
    {
      if (DETECTCHAR (*asrc, mask))
        break;
      asrc++;
    }

  /* We have the matching word, now we resort to a bytewise loop. */

  src = (unsigned char *) asrc;

#endif // !PREFER_SIZE_OVER_SPEED && !__OPTIMIZE_SIZE__

  while (1)
    {
      if (*src == d)
        return (void *) src;
      src++;
    }
}
