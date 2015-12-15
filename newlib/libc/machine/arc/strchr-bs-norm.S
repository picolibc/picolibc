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

/* ARC700 has a relatively long pipeline and branch prediction, so we want
   to avoid branches that are hard to predict.  On the other hand, the
   presence of the norm instruction makes it easier to operate on whole
   words branch-free.  */
#include "asm.h"

#if (defined (__ARC700__) || defined (__ARCEM__) || defined (__ARCHS__)) \
    && defined (__ARC_NORM__) && defined (__ARC_BARREL_SHIFTER__)

ENTRY (strchr)
	extb_s	r1,r1
	asl	r5,r1,8
	bmsk	r2,r0,1
	or	r5,r5,r1
	mov_s	r3,0x01010101
	breq.d	r2,r0,.Laligned
	asl	r4,r5,16
	sub_s	r0,r0,r2
	asl	r7,r2,3
	ld_s	r2,[r0]
#ifdef __LITTLE_ENDIAN__
	asl	r7,r3,r7
#else
	lsr	r7,r3,r7
#endif
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
	and	r7,r12,r4
	breq	r7,0,.Loop ; For speed, we want this branch to be unaligned.
	b_l	.Lfound_char ; Likewise this one.
#else
	and	r12,r12,r4
	breq_l	r12,0,.Loop ; For speed, we want this branch to be unaligned.
	lsr_s	r12,r12,7
	bic	r2,r7,r6
	b.d	.Lfound_char_b
	and_s	r2,r2,r12
#endif
; /* We require this code address to be unaligned for speed...  */
.Laligned:
	ld_s	r2,[r0]
	or	r5,r5,r4
	ror	r4,r3
; /* ... so that this code address is aligned, for itself and ...  */
.Loop:
	sub	r12,r2,r3
	bic_s	r12,r12,r2
	and	r12,r12,r4
	brne.d	r12,0,.Lfound0
	xor	r6,r2,r5
	ld.a	r2,[r0,4]
	sub	r12,r6,r3
	bic	r12,r12,r6
	and	r7,r12,r4
	breq	r7,0,.Loop /* ... so that this branch is unaligned.  */
	; Found searched-for character.  r0 has already advanced to next word.
#ifdef __LITTLE_ENDIAN__
/* We only need the information about the first matching byte
   (i.e. the least significant matching byte) to be exact,
   hence there is no problem with carry effects.  */
.Lfound_char:
	sub	r3,r7,1
	bic	r3,r3,r7
	norm	r2,r3
	sub_s	r0,r0,1
	asr_s	r2,r2,3
	j_l.d	[blink]
	sub_s	r0,r0,r2

	.balign	4
.Lfound0_ua:
	mov_l	r3,r7
.Lfound0:
	sub	r3,r6,r3
	bic	r3,r3,r6
	and	r2,r3,r4
	or_s	r12,r12,r2
	sub_s	r3,r12,1
	bic_s	r3,r3,r12
	norm	r3,r3
	add_s	r0,r0,3
	asr_s	r12,r3,3
	asl.f	0,r2,r3
	sub_s	r0,r0,r12
	j_s.d	[blink]
	mov.pl	r0,0
#else /* BIG ENDIAN */
.Lfound_char:
	lsr	r7,r7,7

	bic	r2,r7,r6
.Lfound_char_b:
	norm	r2,r2
	sub_s	r0,r0,4
	asr_s	r2,r2,3
	j_l.d	[blink]
	add_s	r0,r0,r2

.Lfound0_ua:
	mov_s	r3,r7
.Lfound0:
	asl_s	r2,r2,7
	or	r7,r6,r4
	bic_s	r12,r12,r2
	sub	r2,r7,r3
	or	r2,r2,r6
	bic	r12,r2,r12
	bic.f	r3,r4,r12
	norm	r3,r3

	add.pl	r3,r3,1
	asr_s	r12,r3,3
	asl.f	0,r2,r3
	add_s	r0,r0,r12
	j_s.d	[blink]
	mov.mi	r0,0
#endif /* ENDIAN */
ENDFUNC (strchr)
#endif /* (__ARC700__ || __ARCEM__ || __ARCHS__) && __ARC_NORM__
    && __ARC_BARREL_SHIFTER__ */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
