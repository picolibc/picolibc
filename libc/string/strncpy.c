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
        <<strncpy>>---counted copy string

INDEX
        strncpy

SYNOPSIS
        #include <string.h>
        char *strncpy(char *restrict <[dst]>, const char *restrict <[src]>,
                      size_t <[length]>);

DESCRIPTION
        <<strncpy>> copies not more than <[length]> characters from
        the string pointed to by <[src]> (including the terminating
        null character) to the array pointed to by <[dst]>.  If the
        string pointed to by <[src]> is shorter than <[length]>
        characters, null characters are appended to the destination
        array until a total of <[length]> characters have been
        written.

RETURNS
        This function returns the initial value of <[dst]>.

PORTABILITY
<<strncpy>> is ANSI C.

<<strncpy>> requires no supporting OS subroutines.

QUICKREF
        strncpy ansi pure
*/

#include <string.h>
#include <limits.h>
#include "local.h"

/*SUPPRESS 560*/
/*SUPPRESS 530*/

#undef strncpy

char *
strncpy(char * __restrict dst0, const char * __restrict src0, size_t count)
{
#if defined(__PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__) \
    || defined(_PICOLIBC_NO_OUT_OF_BOUNDS_READS)
    char       *dscan;
    const char *sscan;

    dscan = dst0;
    sscan = src0;
    while (count > 0) {
        --count;
        if ((*dscan++ = *sscan++) == '\0')
            break;
    }
    while (count-- > 0)
        *dscan++ = '\0';

    return dst0;
#else
    char       *dst = dst0;
    const char *src = src0;
    long       *aligned_dst;
    const long *aligned_src;

    /* If SRC and DEST is aligned and count large enough, then copy words.  */
    if (!UNALIGNED_X_Y(src, dst) && !TOO_SMALL_LITTLE_BLOCK(count)) {
        aligned_dst = (long *)dst;
        aligned_src = (long *)src;

        /* SRC and DEST are both "long int" aligned, try to do "long int"
           sized copies.  */
        while (!TOO_SMALL_LITTLE_BLOCK(count) && !DETECT_NULL(*aligned_src)) {
            count -= sizeof(long int);
            *aligned_dst++ = *aligned_src++;
        }

        dst = (char *)aligned_dst;
        src = (char *)aligned_src;
    }

    while (count > 0) {
        --count;
        if ((*dst++ = *src++) == '\0')
            break;
    }

    while (count-- > 0)
        *dst++ = '\0';

    return dst0;
#endif /* not __PREFER_SIZE_OVER_SPEED */
}
