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
        <<memset>>---set an area of memory

INDEX
        memset

SYNOPSIS
        #include <string.h>
        void *memset(void *<[dst]>, int <[c]>, size_t <[length]>);

DESCRIPTION
        This function converts the argument <[c]> into an unsigned
        char and fills the first <[length]> characters of the array
        pointed to by <[dst]> to the value.

RETURNS
        <<memset>> returns the value of <[dst]>.

PORTABILITY
<<memset>> is ANSI C.

    <<memset>> requires no supporting OS subroutines.

QUICKREF
        memset ansi pure
*/

#include <string.h>
#include <stdint.h>
#include "local.h"

#undef memset

void *__no_builtin
memset(void *m, int c, size_t n)
{
    char *s = (char *)m;

#if !defined(__PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
    unsigned int   i;
    unsigned long  buffer;
    unsigned long *aligned_addr;
    unsigned int   d = c & 0xff; /* To avoid sign extension, copy C to an
                                    unsigned variable.  */

    while (UNALIGNED_X(s)) {
        if (n--)
            *s++ = (char)c;
        else
            return m;
    }

    if (!TOO_SMALL_LITTLE_BLOCK(n)) {
        /* If we get this far, we know that n is large and s is word-aligned. */
        aligned_addr = (unsigned long *)s;

        /* Store D into each char sized location in BUFFER so that
           we can set large blocks quickly.  */
        buffer = (d << 8) | d;
        buffer |= (buffer << 16);
        for (i = 32; i < sizeof(buffer) * 8; i <<= 1)
            buffer = (buffer << i) | buffer;

        /* Unroll the loop.  */
        while (!TOO_SMALL_BIG_BLOCK(n)) {
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            n -= BIG_BLOCK_SIZE;
        }

        while (!TOO_SMALL_LITTLE_BLOCK(n)) {
            *aligned_addr++ = buffer;
            n -= LITTLE_BLOCK_SIZE;
        }
        /* Pick up the remainder with a bytewise loop.  */
        s = (char *)aligned_addr;
    }

#endif /* not __PREFER_SIZE_OVER_SPEED */

    while (n--)
        *s++ = (char)c;

    return m;
}
