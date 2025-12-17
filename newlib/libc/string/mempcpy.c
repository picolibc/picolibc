/*
Copyright (c) 2002 Jeff Johnston <jjohnstn@redhat.com>
 */
/*
FUNCTION
        <<mempcpy>>---copy memory regions and return end pointer

SYNOPSIS
        #include <string.h>
        void* mempcpy(void *<[out]>, const void *<[in]>, size_t <[n]>);

DESCRIPTION
        This function copies <[n]> bytes from the memory region
        pointed to by <[in]> to the memory region pointed to by
        <[out]>.

        If the regions overlap, the behavior is undefined.

RETURNS
        <<mempcpy>> returns a pointer to the byte following the
        last byte copied to the <[out]> region.

PORTABILITY
<<mempcpy>> is a GNU extension.

<<mempcpy>> requires no supporting OS subroutines.

        */

#define _GNU_SOURCE
#include <stddef.h>
#include <limits.h>
#include <string.h>
#include "local.h"

void *
mempcpy(void *dst0, const void *src0, size_t len0)
{
#if defined(__PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
    char *dst = (char *)dst0;
    char *src = (char *)src0;

    while (len0--) {
        *dst++ = *src++;
    }

    return dst;
#else
    char       *dst = dst0;
    const char *src = src0;
    long       *aligned_dst;
    const long *aligned_src;

    /* If the size is small, or either SRC or DST is unaligned,
       then punt into the byte copy loop.  This should be rare.  */
    if (!TOO_SMALL_LITTLE_BLOCK(len0) && !UNALIGNED_X_Y(src, dst)) {
        aligned_dst = (long *)dst;
        aligned_src = (long *)src;

        /* Copy 4X long words at a time if possible.  */
        while (!TOO_SMALL_BIG_BLOCK(len0)) {
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            len0 -= BIG_BLOCK_SIZE;
        }

        /* Copy one long word at a time if possible.  */
        while (!TOO_SMALL_LITTLE_BLOCK(len0)) {
            *aligned_dst++ = *aligned_src++;
            len0 -= LITTLE_BLOCK_SIZE;
        }

        /* Pick up any residual with a byte copier.  */
        dst = (char *)aligned_dst;
        src = (char *)aligned_src;
    }

    while (len0--)
        *dst++ = *src++;

    return dst;
#endif /* not __PREFER_SIZE_OVER_SPEED */
}
