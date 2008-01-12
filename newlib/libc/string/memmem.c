/* Byte-wise substring search, using the Two-Way algorithm.
 * Copyright (C) 2008 Eric Blake
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

/*
FUNCTION
	<<memmem>>---find memory segment

INDEX
	memmem

ANSI_SYNOPSIS
	#include <string.h>
	char *memmem(const void *<[s1]>, size_t <[l1]>, const void *<[s2]>,
		     size_t <[l2]>);

DESCRIPTION

	Locates the first occurrence in the memory region pointed to
	by <[s1]> with length <[l1]> of the sequence of bytes pointed
	to by <[s2]> of length <[l2]>.  If you already know the
	lengths of your haystack and needle, <<memmem>> can be much
	faster than <<strstr>>.

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

#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
# define RETURN_TYPE void *
# define AVAILABLE(h, h_l, j, n_l) ((j) <= (h_l) - (n_l))
# include "str-two-way.h"
#endif

void *
_DEFUN (memmem, (haystack_start, haystack_len, needle_start, needle_len),
	const void *haystack_start _AND
	size_t haystack_len _AND
	const void *needle_start _AND
	size_t needle_len)
{
  /* Abstract memory is considered to be an array of 'unsigned char' values,
     not an array of 'char' values.  See ISO C 99 section 6.2.6.1.  */
  const unsigned char *haystack = (const unsigned char *) haystack_start;
  const unsigned char *needle = (const unsigned char *) needle_start;

  if (needle_len == 0)
    /* The first occurrence of the empty string is deemed to occur at
       the beginning of the string.  */
    return (void *) haystack;

#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)

  /* Less code size, but quadratic performance in the worst case.  */
  while (needle_len <= haystack_len)
    {
      if (!memcmp (haystack, needle, needle_len))
        return (void *) haystack;
      haystack++;
      haystack_len--;
    }
  return NULL;

#else /* compilation for speed */

  /* Larger code size, but guaranteed linear performance.  */

  /* Sanity check, otherwise the loop might search through the whole
     memory.  */
  if (haystack_len < needle_len)
    return NULL;

  /* Use optimizations in memchr when possible, to reduce the search
     size of haystack using a linear algorithm with a smaller
     coefficient.  However, avoid memchr for long needles, since we
     can often achieve sublinear performance.  */
  if (needle_len < LONG_NEEDLE_THRESHOLD)
    {
      haystack = memchr (haystack, *needle, haystack_len);
      if (!haystack || needle_len == 1)
	return (void *) haystack;
      haystack_len -= haystack - (const unsigned char *) haystack_start;
      if (haystack_len < needle_len)
	return NULL;
      return two_way_short_needle (haystack, haystack_len, needle, needle_len);
    }
  return two_way_long_needle (haystack, haystack_len, needle, needle_len);
#endif /* compilation for speed */
}
