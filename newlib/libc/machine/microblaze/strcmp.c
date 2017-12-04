/* Copyright (c) 2009 Xilinx, Inc.  All rights reserved. 

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
   
   1.  Redistributions source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 
   
   2.  Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution. 
   
   3.  Neither the name of Xilinx nor the names of its contributors may be
   used to endorse or promote products derived from this software without
   specific prior written permission. 
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
  
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

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

/* DETECTNULL returns nonzero if (long)X contains a NULL byte. */
#if LONG_MAX == 2147483647L
#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#else
#if LONG_MAX == 9223372036854775807L
#define DETECTNULL(X) (((X) - 0x0101010101010101) & ~(X) & 0x8080808080808080)
#else
#error long int is not a 32bit or 64bit type.
#endif
#endif

#ifndef DETECTNULL
#error long int is not a 32bit or 64bit byte
#endif

int
strcmp (const char *s1,
	const char *s2)
{ 

#ifndef HAVE_HW_PCMP

#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
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
  if (!UNALIGNED (s1, s2))
    {  
      /* If s1 and s2 are word-aligned, compare them a word at a time. */
      a1 = (unsigned long*)s1;
      a2 = (unsigned long*)s2;
      while (*a1 == *a2)
        {
          /* To get here, *a1 == *a2, thus if we find a null in *a1,
	     then the strings must be equal, so return zero.  */
          if (DETECTNULL (*a1))
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
#endif /* not PREFER_SIZE_OVER_SPEED */

#else

#include "mb_endian.h"

    asm volatile ("                                          \n\
        or      r9, r0, r0               /* Index register */\n\
check_alignment:                                             \n\
        andi    r3, r5, 3                                    \n\
        andi    r4, r6, 3                                    \n\
        bnei    r3, try_align_args                           \n\
        bnei    r4, regular_strcmp     /* At this point we don't have a choice */ \n\
cmp_loop:                                                                       \n"
        LOAD4BYTES("r3", "r5", "r9")
        LOAD4BYTES("r4", "r6", "r9")
"                                                                                      \n\
        pcmpbf  r7, r3, r0              /* See if there is Null byte */                         \n\
        bnei    r7, end_cmp_loop        /* IF yes (r7 > 0) use byte compares in end_cmp_loop */ \n\
        cmpu    r7, r4, r3              /* ELSE compare whole word */                   \n\
        bnei    r7, end_cmp                                                             \n\
        brid    cmp_loop                                                                \n\
        addik   r9, r9, 4               /* delay slot */                                \n\
end_cmp_loop:                                                                           \n\
        lbu     r3, r5, r9              /* byte compare loop */                         \n\
        lbu     r4, r6, r9                                                              \n\
        cmpu    r7, r4, r3              /* Compare bytes */                             \n\
        bnei    r7, end_cmp_early                                                       \n\
        bneid   r3, end_cmp_loop        /* If reached null on one string, terminate */  \n\
        addik   r9, r9, 1               /* delay slot */                        \n\
end_cmp_early:                                                                  \n\
        rtsd    r15, 8                                                          \n\
        or      r3, r0, r7                                                      \n\
try_align_args:                                                                 \n\
        xor     r7, r4, r3                                                      \n\
        bnei    r7, regular_strcmp      /* cannot align args */                 \n\
        rsubik  r10, r3, 4              /* Number of initial bytes to align */  \n\
align_loop:                                                                     \n\
        lbu     r3, r5, r9                                                      \n\
        lbu     r4, r6, r9                                                      \n\
        cmpu    r7, r4, r3                                                      \n\
        bnei    r7, end_cmp                                                     \n\
        beqi    r3, end_cmp                                                     \n\
        addik   r10, r10, -1                                                    \n\
        beqid   r10, cmp_loop                                                   \n\
        addik   r9, r9, 1                                                       \n\
        bri     align_loop                                                      \n\
regular_strcmp:                                                                 \n\
        lbu     r3, r5, r9                                                      \n\
        lbu     r4, r6, r9                                                      \n\
        cmpu    r7, r4, r3                                                      \n\
        bnei    r7, end_cmp                                                     \n\
        beqi    r3, end_cmp                                                     \n\
        brid    regular_strcmp                                                  \n\
        addik   r9, r9, 1                                                       \n\
end_cmp:                                                                        \n\
        rtsd    r15, 8                                                          \n\
        or      r3, r0, r7              /* Return strcmp result */");

#endif /* ! HAVE_HW_PCMP */
}

