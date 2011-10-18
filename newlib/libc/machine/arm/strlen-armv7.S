/* Copyright (c) 2010-2011, Linaro Limited
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

      * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

      * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

      * Neither the name of Linaro Limited nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   Written by Dave Gilbert <david.gilbert@linaro.org>

   This strlen routine is optimised on a Cortex-A9 and should work on
   all ARMv7 processors.   This routine is reasonably fast for short
   strings, but is probably slower than a simple implementation if all
   your strings are very short */

@ 2011-02-08 david.gilbert@linaro.org
@    Extracted from local git 6848613a
@ 2011-10-13 david.gilbert@linaro.org
@    Extracted from cortex-strings bzr rev 63
@      Integrate to newlib, flip to ldrd
@      Pull in Endian macro from my memchr

#include "arm_asm.h"

@ NOTE: This ifdef MUST match the ones in arm/strlen.c
@ We fallback to the one in arm/strlen.c for size optimised or
@ for older arch's
#if defined(_ISA_ARM_7) || defined(__ARM_ARCH_6T2__) && \
    !(defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED) || \
      (defined (__thumb__) && !defined (__thumb2__)))

@ this lets us check a flag in a 00/ff byte easily in either endianness
#ifdef __ARMEB__
#define CHARTSTMASK(c) 1<<(31-(c*8))
#else
#define CHARTSTMASK(c) 1<<(c*8)
#endif

@------------------------------------------------------------------------------
	.syntax unified
	.arch armv7-a

	.thumb_func
	.align 2
	.p2align 4,,15
	.global strlen
	.type strlen,%function
strlen:
	@ r0 = string
	@ returns count of bytes in string not including terminator
	mov	r1, r0
	push	{ r4,r6 }
	mvns	r6, #0		@ all F
	movs	r4, #0
	tst	r0, #7
	beq	2f

1:
	ldrb	r2, [r1], #1
	tst	r1, #7		@ Hit alignment yet?
	cbz	r2, 10f		@ Exit if we found the 0
	bne	1b

	@ So we're now aligned
2:
	ldrd    r2,r3,[r1],#8
	uadd8	r2, r2, r6	@ Par add 0xff - sets the GE bits for bytes!=0
	sel	r2, r4, r6	@ bytes are 00 for none-00 bytes,
				@ or ff for 00 bytes - NOTE INVERSION
	uadd8	r3, r3, r6	@ Par add 0xff - sets the GE bits for bytes!=0
	sel	r3, r2, r6	@ chained...bytes are 00 for none-00 bytes,
				@ or ff for 00 bytes - NOTE INVERSION
	cmp	r3, #0
	beq	2b

strlenendtmp:
	@ One (or more) of the bytes we loaded was 0 - but which one?
	@ r2 has the mask corresponding to the first loaded word
	@ r3 has a combined mask of the two words - but if r2 was all-non 0 
	@ then it's just the 2nd words
	cmp	r2, #0
	itte	eq
	moveq	r2, r3		@ the end is in the 2nd word
	subeq	r1,r1,#3
	subne	r1,r1,#7

	@ r1 currently points to the 2nd byte of the word containing the 0
	tst	r2, # CHARTSTMASK(0)	@ 1st character
	bne	10f
	adds	r1,r1,#1
	tst	r2, # CHARTSTMASK(1)	@ 2nd character
	ittt	eq
	addeq	r1,r1,#1
	tsteq	r2, # (3<<15)	@ 2nd & 3rd character
	@ If not the 3rd must be the last one
	addeq	r1,r1,#1

10:
	@ r0 is still at the beginning, r1 is pointing 1 byte after the nul
	sub	r0, r1, r0
	subs	r0, r0, #1
	pop	{ r4, r6 }
	bx	lr

#endif
