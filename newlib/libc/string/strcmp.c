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
	<<strcmp>>---character string compare

INDEX
	strcmp

SYNOPSIS
	#include <string.h>
	int strcmp(const char *<[a]>, const char *<[b]>);

DESCRIPTION
	<<strcmp>> compares the string at <[a]> to
	the string at <[b]>.

RETURNS
	If <<*<[a]>>> sorts lexicographically after <<*<[b]>>>,
	<<strcmp>> returns a number greater than zero.  If the two
	strings match, <<strcmp>> returns zero.  If <<*<[a]>>>
	sorts lexicographically before <<*<[b]>>>, <<strcmp>> returns a
	number less than zero.

PORTABILITY
<<strcmp>> is ANSI C.

<<strcmp>> requires no supporting OS subroutines.

QUICKREF
	strcmp ansi pure
*/

#include <string.h>
#include <limits.h>
#include "local.h"

int
strcmp (const char *s1,
	const char *s2)
{
#if ((defined(__PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)) && !defined(__FAST_STRCMP)) \
    || defined(_PICOLIBC_NO_OUT_OF_BOUNDS_READS)
  while (*s1 != '\0' && *s1 == *s2)
    {
      s1++;
      s2++;
    }

  return (*(unsigned char *) s1) - (*(unsigned char *) s2);
#else
  unsigned long *a1;
  unsigned long *a2;

  /* If s1 or s2 are unaligned, then compare bytes. */
  if (!UNALIGNED_X_Y(s1, s2))
    {
      /* If s1 and s2 are word-aligned, compare them a word at a time. */
      a1 = (unsigned long*)s1;
      a2 = (unsigned long*)s2;
      while (*a1 == *a2)
        {
          /* To get here, *a1 == *a2, thus if we find a null in *a1,
	     then the strings must be equal, so return zero.  */
          if (DETECT_NULL(*a1))
	    return 0;

          a1++;
          a2++;
        }

      /* A difference was detected in last few bytes of s1, so search bytewise */
      s1 = (char*)a1;
      s2 = (char*)a2;
    }

  while (*s1 != '\0' && *s1 == *s2)
    {
      s1++;
      s2++;
    }
  return (*(unsigned char *) s1) - (*(unsigned char *) s2);
#endif /* not __PREFER_SIZE_OVER_SPEED */
}
