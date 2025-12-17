/*
Copyright (c) 2007 Corinna Vinschen <corinna@vinschen.de>
 */
/*
FUNCTION
        <<stpncpy>>---counted copy string returning a pointer to its end

INDEX
        stpncpy

SYNOPSIS
        #include <string.h>
        char *stpncpy(char *restrict <[dst]>, const char *restrict <[src]>,
                      size_t <[length]>);

DESCRIPTION
        <<stpncpy>> copies not more than <[length]> characters from the
        the string pointed to by <[src]> (including the terminating
        null character) to the array pointed to by <[dst]>.  If the
        string pointed to by <[src]> is shorter than <[length]>
        characters, null characters are appended to the destination
        array until a total of <[length]> characters have been
        written.

RETURNS
        This function returns a pointer to the end of the destination string,
        thus pointing to the trailing '\0', or, if the destination string is
        not null-terminated, pointing to dst + n.

PORTABILITY
<<stpncpy>> is a GNU extension, candidate for inclusion into POSIX/SUSv4.

<<stpncpy>> requires no supporting OS subroutines.

QUICKREF
        stpncpy gnu
*/

#define _DEFAULT_SOURCE
#include <string.h>
#include <limits.h>
#include "local.h"

/*SUPPRESS 560*/
/*SUPPRESS 530*/

char *
stpncpy(char * __restrict dst, const char * __restrict src, size_t count)
{
    char *ret = NULL;

#if !defined(__PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__) \
    && !defined(_PICOLIBC_NO_OUT_OF_BOUNDS_READS)
    long       *aligned_dst;
    const long *aligned_src;

    /* If SRC and DEST is aligned and count large enough, then copy words.  */
    if (!UNALIGNED_X_Y(src, dst) && !TOO_SMALL_LITTLE_BLOCK(count)) {
        aligned_dst = (long *)dst;
        aligned_src = (long *)src;

        /* SRC and DEST are both LITTLE_BLOCK_SIZE aligned,
           try to do LITTLE_BLOCK_SIZE sized copies.  */
        while (!TOO_SMALL_LITTLE_BLOCK(count) && !DETECT_NULL(*aligned_src)) {
            count -= LITTLE_BLOCK_SIZE;
            *aligned_dst++ = *aligned_src++;
        }

        dst = (char *)aligned_dst;
        src = (char *)aligned_src;
    }
#endif /* not __PREFER_SIZE_OVER_SPEED */

    while (count > 0) {
        --count;
        if ((*dst++ = *src++) == '\0') {
            ret = dst - 1;
            break;
        }
    }

    while (count-- > 0)
        *dst++ = '\0';

    return ret ? ret : dst;
}
