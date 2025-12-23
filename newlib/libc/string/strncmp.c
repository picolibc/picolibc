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
        <<strncmp>>---character string compare

INDEX
        strncmp

SYNOPSIS
        #include <string.h>
        int strncmp(const char *<[a]>, const char * <[b]>, size_t <[length]>);

DESCRIPTION
        <<strncmp>> compares up to <[length]> characters
        from the string at <[a]> to the string at <[b]>.

RETURNS
        If <<*<[a]>>> sorts lexicographically after <<*<[b]>>>,
        <<strncmp>> returns a number greater than zero.  If the two
        strings are equivalent, <<strncmp>> returns zero.  If <<*<[a]>>>
        sorts lexicographically before <<*<[b]>>>, <<strncmp>> returns a
        number less than zero.

PORTABILITY
<<strncmp>> is ANSI C.

<<strncmp>> requires no supporting OS subroutines.

QUICKREF
        strncmp ansi pure
*/

#include <string.h>
#include <limits.h>
#include "local.h"

int
strncmp(const char *s1, const char *s2, size_t n)
{
#if defined(__PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__) \
    || defined(_PICOLIBC_NO_OUT_OF_BOUNDS_READS)
    if (n == 0)
        return 0;

    while (n-- != 0 && *s1 == *s2) {
        if (n == 0 || *s1 == '\0')
            break;
        s1++;
        s2++;
    }

    return (*(unsigned char *)s1) - (*(unsigned char *)s2);
#else
    unsigned long *a1;
    unsigned long *a2;

    if (n == 0)
        return 0;

    /* If s1 or s2 are unaligned, then compare bytes. */
    if (!UNALIGNED_X_Y(s1, s2)) {
        /* If s1 and s2 are word-aligned, compare them a word at a time. */
        a1 = (unsigned long *)s1;
        a2 = (unsigned long *)s2;
        while (n >= sizeof(long) && *a1 == *a2) {
            n -= sizeof(long);

            /* If we've run out of bytes or hit a null, return zero
               since we already know *a1 == *a2.  */
            if (n == 0 || DETECT_NULL(*a1))
                return 0;

            a1++;
            a2++;
        }

        /* A difference was detected in last few bytes of s1, so search bytewise */
        s1 = (char *)a1;
        s2 = (char *)a2;
    }

    while (n-- > 0 && *s1 == *s2) {
        /* If we've run out of bytes or hit a null, return zero
           since we already know *s1 == *s2.  */
        if (n == 0 || *s1 == '\0')
            return 0;
        s1++;
        s2++;
    }
    return (*(unsigned char *)s1) - (*(unsigned char *)s2);
#endif /* not __PREFER_SIZE_OVER_SPEED */
}
