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
	<<strlen>>---character string length
	
INDEX
	strlen

ANSI_SYNOPSIS
	#include <string.h>
	size_t strlen(const char *<[str]>);

TRAD_SYNOPSIS
	#include <string.h>
	size_t strlen(<[str]>)
	char *<[src]>;

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

#define LBLOCKSIZE   (sizeof (long))
#define UNALIGNED(X) ((long)X & (LBLOCKSIZE - 1))

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
_DEFUN (strlen, (str),
	_CONST char *str)
{

#ifndef HAVE_HW_PCMP

#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  _CONST char *start = str;

  while (*str)
    str++;

  return str - start;
#else
  _CONST char *start = str;
  unsigned long *aligned_addr;

  if (!UNALIGNED (str))
    {
      /* If the string is word-aligned, we can check for the presence of 
         a null in each word-sized block.  */
      aligned_addr = (unsigned long*)str;
      while (!DETECTNULL (*aligned_addr))
        aligned_addr++;

      /* Once a null is detected, we check each byte in that block for a
         precise position of the null.  */
      str = (char*)aligned_addr;
    }
 
  while (*str)
    str++;
  return str - start;
#endif /* not PREFER_SIZE_OVER_SPEED */

#else

#include "mb_endian.h"

  asm volatile ("                                               \n\
        or      r9, r0, r0              /* Index register */    \n\
check_alignment:                                                \n\
        andi    r3, r5, 3                                       \n\
        bnei    r3, align_arg                                   \n\
len_loop:                                                       \n"
        LOAD4BYTES("r3", "r5", "r9")
"                                                               \n\
        pcmpbf  r4, r3, r0                                      \n\
        bnei    r4, end_len                                     \n\
        brid    len_loop                                        \n\
        addik   r9, r9, 4                                       \n\
end_len:                                                        \n\
        lbu     r3, r5, r9                                      \n\
        beqi    r3, done_len                                    \n\
        brid    end_len                                         \n\
        addik   r9, r9, 1                                       \n\
done_len:                                                       \n\
        rtsd    r15, 8                                          \n\
        or      r3, r0, r9              /* Return len */        \n\
align_arg:                                                      \n\
        rsubik  r10, r3, 4                                      \n\
align_loop:                                                     \n\
        lbu     r3, r5, r9                                      \n\
        beqid   r3, done_len                                    \n\
        addik   r10, r10, -1                                    \n\
        bneid   r10, align_loop                                 \n\
        addik   r9, r9, 1                                       \n\
        bri     len_loop");

#endif  /* ! HAVE_HW_PCMP */
}
