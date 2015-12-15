/*
   Copyright (c) 2015, Synopsys, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1) Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2) Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   3) Neither the name of the Synopsys, Inc., nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

/* This implementation is optimized for performance.  For code size a generic
   implementation of this function from newlib/libc/string/strchr.c will be
   used.  */
#if !defined (__OPTIMIZE_SIZE__) && !defined (PREFER_SIZE_OVER_SPEED)

#include "asm.h"

/* In order to search for a zero in a W, we calculate
   X := (W - 0x01010101) & ~W & 0x80808080;
   In the little endian case:
     If no byte in W is zero, X will be zero; otherwise, the least significant
     byte of X which is nonzero indicates the least significant byte of W that
     is zero.
   In the big endian case:
     X will be zero iff no byte in W is zero.
     If X is nonzero, to find out which is the most significant zero byte
     in W, we calculate:
     Y := ~(((W | 0x80808080) - 0x01010101) | W) & 0x80808080;
     Each byte in Y is 0x80 if the the corresponding byte in
     W is zero, otherwise that byte of Y is 0.  */

#if defined (__ARC_BARREL_SHIFTER__) && \
   (defined (__ARC600__) || (!defined (__ARC_NORM__) && !defined (__ARC601__)))

ENTRY (strchr)
	bmsk.f	r2,r0,1
	mov_s	r3,0x01010101
	extb_s	r1,r1
	asl	r5,r1,8
	or	r5,r5,r1
	beq.d	.Laligned
	asl	r4,r5,16
        sub_s   r0,r0,r2
        asl_s   r2,r2,3
#ifdef	__LITTLE_ENDIAN__
        asl     r7,r3,r2
#else
        lsr     r7,r3,r2
#endif
	ld_s	r2,[r0]
	or	r5,r5,r4
	ror	r4,r3
	sub	r12,r2,r7
	bic_s	r12,r12,r2
	and	r12,r12,r4

	brne.d	r12,0,.Lfound0_ua
	xor	r6,r2,r5
	ld.a	r2,[r0,4]
	sub	r12,r6,r7
	bic	r12,r12,r6
#ifdef __LITTLE_ENDIAN__
	and.f	r7,r12,r4
	sub	r12,r2,r3
	bic_s	r12,r12,r2
	beq.d	.Loop
	and	r12,r12,r4
	b.d	.Lfound_char_ua
	btst	r7,7
#else
	and.f	r8,r12,r4
	sub	r12,r2,r3
	bic_s	r12,r12,r2
	beq.d	.Loop
	and	r12,r12,r4
	bic	r12,r7,r6
	asl_s	r12,r12,7
	and.f	r2,r8,r12
	b.d	.Lfound_char_ua
	sub_s	r0,r0,4
#endif

	.balign	4
.Laligned:
	ld_s	r2,[r0]
	or	r5,r5,r4
	ror	r4,r3
	sub	r12,r2,r3
	bic_s	r12,r12,r2
	and	r12,r12,r4
.Loop:

	brne.d	r12,0,.Lfound0
	xor	r6,r2,r5
	ld.a	r2,[r0,4]
	sub	r12,r6,r3
	bic	r12,r12,r6
	and.f	r7,r12,r4
	sub	r12,r2,r3
	bic_s	r12,r12,r2
	beq.d	.Loop
	and	r12,r12,r4
; Found searched-for character.  r0 has already advanced to next word.
#ifdef __LITTLE_ENDIAN__
/* We only need the information about the first matching byte
   (i.e. the least significant matching byte) to be exact,
   hence there is no problem with carry effects.  */
.Lfound_char:
	btst	r7,7
.Lfound_char_ua:
	sub_s	r0,r0,4
	add.eq	r0,r0,1
	btst.eq	r7,15
	add.eq	r0,r0,1
	btst.eq	r7,23
	j_s.d	[blink]
	add.eq	r0,r0,1

	.balign	4
.Lfound0_ua:
	mov_l	r3,r7
.Lfound0:
	sub	r2,r6,r3
	bic	r2,r2,r6
	and	r2,r2,r4
	or	r3,r12,r2
	sub_s	r12,r3,1
	xor_s	r3,r3,r12
	tst_s	r2,r3
	lsr	r2,r3,31
	lsr	r12,r3,16
	jeq.d	[blink]
	mov.eq	r0,0
	lsr	r3,r3,8
	sub_s	r2,r2,r12
	sub_s	r2,r2,r3
	bmsk_s	r2,r2,1
	j_s.d	[blink]
	add_s	r0,r0,r2
#else /* BIG ENDIAN */
.Lfound_char:
	asl	r6,r6,7
	sub_s	r0,r0,4
	bic.f	r2,r7,r6
.Lfound_char_ua:
	add.pl	r0,r0,1
	jmi.d	[blink]
	btst_s	r2,23
	add.eq	r0,r0,1
	btst.eq	r2,15
	j_s.d	[blink]
	add.eq	r0,r0,1

; N.B. if we searched for a char zero and found it in the MSB,
; and ignored matches are identical, we will take the early exit
; like for an ordinary found zero - except for the extra stalls at jhi -
; but still compute the right result.
.Lfound0_ua:
	mov_s	r3,r7
.Lfound0:
	asl_s	r2,r2,7
	or	r7,r6,r4
	bic_s	r12,r12,r2
	sub	r2,r7,r3
	or	r2,r2,r6
	bic	r2,r4,r2
	cmp	r12,r2
	mov.hi	r0,0
	btst.ls	r2,31
	jhi.d	[blink]
	add.eq	r0,r0,1
	btst.eq	r2,23
	add.eq	r0,r0,1
	btst.eq	r2,15
	j_s.d	[blink]
	add.eq	r0,r0,1
#endif /* ENDIAN */
ENDFUNC (strchr)
#endif /* __ARC_BARREL_SHIFTER__ &&
    (__ARC600__ || (!__ARC_NORM__ && !__ARC601__)) */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
