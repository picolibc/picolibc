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
	<<strlen>>---character string length

INDEX
	strlen

SYNOPSIS
	#include <string.h>
	size_t strlen(const char *<[str]>);

DESCRIPTION
	The <<strlen>> function works out the length of the string
	starting at <<*<[str]>>> by counting chararacters until it
	reaches a <<NULL>> character.

RETURNS
	<<strlen>> returns the character count.

PORTABILITY
<<strlen>> is ANSI C.

<<strlen>> requires no supporting OS subroutines.

QUICKREF
	strlen ansi pure
*/

#include <_ansi.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#define LBLOCKSIZE   (sizeof (long))
#define UNALIGNED(X) ((uintptr_t)X & (LBLOCKSIZE - 1))

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

size_t
strlen (const char *str)
{
  const char *start = str;

#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__) && \
    !defined(PICOLIBC_NO_OUT_OF_BOUNDS_READS)
  unsigned long *aligned_addr;

  /* Align the pointer, so we can search a word at a time.  */
  while (UNALIGNED (str))
    {
      if (!*str)
	return str - start;
      str++;
    }

  /* If the string is word-aligned, we can check for the presence of
     a null in each word-sized block.  */
  aligned_addr = (unsigned long *)str;
  while (!DETECTNULL (*aligned_addr))
    aligned_addr++;

  /* Once a null is detected, we check each byte in that block for a
     precise position of the null.  */
  str = (char *) aligned_addr;

#endif /* not PREFER_SIZE_OVER_SPEED */

  while (*str)
    str++;
  return str - start;
}
