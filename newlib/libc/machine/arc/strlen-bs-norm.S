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
   implementation of this function from newlib/libc/string/strlen.c will be
   used.  */
#if !defined (__OPTIMIZE_SIZE__) && !defined (PREFER_SIZE_OVER_SPEED)

#include "asm.h"
#if (defined (__ARC700__) || defined (__ARCEM__) || defined (__ARCHS__)) \
    && defined (__ARC_NORM__) && defined (__ARC_BARREL_SHIFTER__)

ENTRY (strlen)
	or	r3,r0,7
	ld	r2,[r3,-7]
	ld.a	r6,[r3,-3]
	mov	r4,0x01010101
	; uses long immediate
#ifdef __LITTLE_ENDIAN__
	asl_s	r1,r0,3
	btst_s	r0,2
	asl	r7,r4,r1
	ror	r5,r4
	sub	r1,r2,r7
	bic_s	r1,r1,r2
	mov.eq	r7,r4
	sub	r12,r6,r7
	bic	r12,r12,r6
	or.eq	r12,r12,r1
	and	r12,r12,r5
	brne	r12,0,.Learly_end
#else /* BIG ENDIAN */
	ror	r5,r4
	btst_s	r0,2
	mov_s	r1,31
	sub3	r7,r1,r0
	sub	r1,r2,r4
	bic_s	r1,r1,r2
	bmsk	r1,r1,r7
	sub	r12,r6,r4
	bic	r12,r12,r6
	bmsk.ne	r12,r12,r7
	or.eq	r12,r12,r1
	and	r12,r12,r5
	brne	r12,0,.Learly_end
#endif /* ENDIAN */

.Loop:
	ld_s	r2,[r3,4]
	ld.a	r6,[r3,8]
	; stall for load result
	sub	r1,r2,r4
	bic_s	r1,r1,r2
	sub	r12,r6,r4
	bic	r12,r12,r6
	or_l	r12,r12,r1
	and	r12,r12,r5
	breq_l	r12,0,.Loop
.Lend:
	and.f	r1,r1,r5
	sub.ne	r3,r3,4
	mov.eq	r1,r12
#ifdef __LITTLE_ENDIAN__
	sub_s	r2,r1,1
	bic_s	r2,r2,r1
	norm	r1,r2
	sub_s	r0,r0,3
	lsr_s	r1,r1,3
	sub	r0,r3,r0
	j_s.d	[blink]
	sub_l	r0,r0,r1
#else /* BIG ENDIAN */
	lsr_s	r1,r1,7
	mov.eq	r2,r6
	bic_s	r1,r1,r2
	norm	r1,r1
	sub	r0,r3,r0
	lsr_s	r1,r1,3
	j_s.d	[blink]
	add_l	r0,r0,r1
#endif /* ENDIAN */
.Learly_end:
	b.d	.Lend
	sub_s.ne r1,r1,r1
ENDFUNC (strlen)
#endif /* (__ARC700__ || __ARCEM__ || __ARCHS__) && __ARC_NORM__
    && __ARC_BARREL_SHIFTER__ */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
