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
        <<strchr>>---search for character in string

INDEX
        strchr

SYNOPSIS
        #include <string.h>
        char * strchr(const char *<[string]>, int <[c]>);

DESCRIPTION
        This function finds the first occurence of <[c]> (converted to
        a char) in the string pointed to by <[string]> (including the
        terminating null character).

RETURNS
        Returns a pointer to the located character, or a null pointer
        if <[c]> does not occur in <[string]>.

PORTABILITY
<<strchr>> is ANSI C.

<<strchr>> requires no supporting OS subroutines.

QUICKREF
        strchr ansi pure
*/

#include <string.h>
#include <limits.h>
#include "local.h"

char *
strchr(const char *s1, int i)
{
    const unsigned char *s = (const unsigned char *)s1;
    unsigned char        c = i;

#if !defined(__PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__) \
    && !defined(_PICOLIBC_NO_OUT_OF_BOUNDS_READS)
    unsigned long  mask, j;
    unsigned long *aligned_addr;

    /* Special case for finding 0.  */
    if (!c) {
        while (UNALIGNED_X(s)) {
            if (!*s)
                return (char *)s;
            s++;
        }
        /* Operate a word at a time.  */
        aligned_addr = (unsigned long *)s;
        while (!DETECT_NULL(*aligned_addr))
            aligned_addr++;
        /* Found the end of string.  */
        s = (const unsigned char *)aligned_addr;
        while (*s)
            s++;
        return (char *)s;
    }

    /* All other bytes.  Align the pointer, then search a long at a time.  */
    while (UNALIGNED_X(s)) {
        if (!*s)
            return NULL;
        if (*s == c)
            return (char *)s;
        s++;
    }

    mask = c;
    for (j = 8; j < sizeof(mask) * 8; j <<= 1)
        mask = (mask << j) | mask;

    aligned_addr = (unsigned long *)s;
    while (!DETECT_NULL(*aligned_addr) && !DETECT_CHAR(*aligned_addr, mask))
        aligned_addr++;

    /* The block of bytes currently pointed to by aligned_addr
       contains either a null or the target char, or both.  We
       catch it using the bytewise search.  */

    s = (unsigned char *)aligned_addr;

#endif /* not __PREFER_SIZE_OVER_SPEED */

    while (*s && *s != c)
        s++;
    if (*s == c)
        return (char *)s;
    return NULL;
}
