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
        <<strcat>>---concatenate strings

INDEX
        strcat

SYNOPSIS
        #include <string.h>
        char *strcat(char *restrict <[dst]>, const char *restrict <[src]>);

DESCRIPTION
        <<strcat>> appends a copy of the string pointed to by <[src]>
        (including the terminating null character) to the end of the
        string pointed to by <[dst]>.  The initial character of
        <[src]> overwrites the null character at the end of <[dst]>.

RETURNS
        This function returns the initial value of <[dst]>

PORTABILITY
<<strcat>> is ANSI C.

<<strcat>> requires no supporting OS subroutines.

QUICKREF
        strcat ansi pure
*/

#include <string.h>
#include <limits.h>
#include "local.h"

/*SUPPRESS 560*/
/*SUPPRESS 530*/

#undef strcat

char *
strcat(char * __restrict s1, const char * __restrict s2)
{
#if defined(__PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__) \
    || defined(_PICOLIBC_NO_OUT_OF_BOUNDS_READS)
    char *s = s1;

    while (*s1)
        s1++;

    while ((*s1++ = *s2++))
        ;
    return s;
#else
    char *s = s1;

    /* Skip unaligned memory in s1.  */
    while (UNALIGNED_X(s1) && *s1)
        s1++;

    if (*s1) {
        /* Skip over the aligned data in s1 as quickly as possible.  */
        unsigned long *aligned_s1 = (unsigned long *)s1;
        while (!DETECT_NULL(*aligned_s1))
            aligned_s1++;
        s1 = (char *)aligned_s1;

        /* Find string terminator.  */
        while (*s1)
            s1++;
    }

    /* s1 now points to the its trailing null character, we can
       just use strcpy to do the work for us now.

       ?!? We might want to just include strcpy here.
       Also, this will cause many more unaligned string copies because
       s1 is much less likely to be aligned.  I don't know if its worth
       tweaking strcpy to handle this better.  */
    strcpy(s1, s2);

    return s;
#endif /* not __PREFER_SIZE_OVER_SPEED */
}
