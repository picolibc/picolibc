/* Copyright (c) 2012 Corinna Vinschen <corinna@vinschen.de> */
/*
FUNCTION
        <<memrchr>>---reverse search for character in memory

INDEX
        memrchr

SYNOPSIS
        #include <string.h>
        void *memrchr(const void *<[src]>, int <[c]>, size_t <[length]>);

DESCRIPTION
        This function searches memory starting at <[length]> bytes
        beyond <<*<[src]>>> backwards for the character <[c]>.
        The search only ends with the first occurrence of <[c]>; in
        particular, <<NUL>> does not terminate the search.

RETURNS
        If the character <[c]> is found within <[length]> characters
        of <<*<[src]>>>, a pointer to the character is returned. If
        <[c]> is not found, then <<NULL>> is returned.

PORTABILITY
<<memrchr>> is a GNU extension.

<<memrchr>> requires no supporting OS subroutines.

QUICKREF
        memrchr
*/

#define _GNU_SOURCE
#include <string.h>
#include <limits.h>
#include "local.h"

void *
memrchr(const void *src_void, int c, size_t length)
{
    const unsigned char *src = (const unsigned char *)src_void + length - 1;
    unsigned char        d = c;

#if !defined(__PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
    unsigned long *asrc;
    unsigned long  mask;
    unsigned int   i;

    while (UNALIGNED_X(src + 1)) {
        if (!length--)
            return NULL;
        if (*src == d)
            return (void *)src;
        src--;
    }

    if (!TOO_SMALL_LITTLE_BLOCK(length)) {
        /* If we get this far, we know that length is large and src is
           word-aligned. */
        /* The fast code reads the source one word at a time and only
           performs the bytewise search on word-sized segments if they
           contain the search character, which is detected by XORing
           the word-sized segment with a word-sized block of the search
           character and then detecting for the presence of NUL in the
           result.  */
        asrc = (unsigned long *)(src - LITTLE_BLOCK_SIZE + 1);
        mask = (unsigned long)d << 8 | (unsigned long)d;
        mask = mask << 16 | mask;
        for (i = 32; i < sizeof(mask) * 8; i <<= 1)
            mask = (mask << i) | mask;

        while (!TOO_SMALL_LITTLE_BLOCK(length)) {
            if (DETECT_CHAR(*asrc, mask))
                break;
            length -= LITTLE_BLOCK_SIZE;
            asrc--;
        }

        /* If there are fewer than LITTLE_BLOCK_SIZE characters left,
           then we resort to the bytewise loop.  */

        src = (unsigned char *)asrc + LITTLE_BLOCK_SIZE - 1;
    }

#endif /* not __PREFER_SIZE_OVER_SPEED */

    while (length--) {
        if (*src == d)
            return (void *)src;
        src--;
    }

    return NULL;
}
