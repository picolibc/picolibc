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

#if defined(__ARC601__) || !defined (__ARC_BARREL_SHIFTER__)
/* This code is optimized for the ARC601 pipeline without barrel shifter.  */

ENTRY (strlen)
	or	r3,r0,7
	ld	r2,[r3,-7]
	ld.a	r6,[r3,-3]
	mov	r4,0x01010101
	; uses long immediate
#ifdef __LITTLE_ENDIAN__
	bmsk.f	0,r0,1
	mov_s	r1,31
	add3_s	r1,r1,r0
	bmsk	r7,r4,r1
	xor.ne	r7,r7,r4
	btst_s	r0,2
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
	add.f	r1,r4,30 ; r1 mod 31 := -1; clear carry
	ror	r5,r4
	sub3	r7,r1,r0
	btst_s	r0,2
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
	or_s	r12,r12,r1
	and	r12,r12,r5
	breq_s	r12,0,.Loop
.Lend:
	and.f	r1,r1,r5
	sub.ne	r3,r3,4
#ifdef __LITTLE_ENDIAN__
	mov.eq	r1,r12
	btst_s	r1,7
	sub	r0,r3,r0
	add.eq	r0,r0,1
	bmsk.f	0,r1,15
	add.eq	r0,r0,1
	bmsk.f	0,r1,23
	j_s.d	[blink]
	add.eq	r0,r0,1
#else /* BIG ENDIAN */
#ifdef __OPTIMIZE_SIZE__
1:	ldb_s	r1,[r3]
	breq_s	r1,0,0f
	ldb.a	r1,[r3,1]
	breq_s	r1,0,0f
	ldb.a	r1,[r3,1]
	breq_s	r1,0,0f
	add_s	r3,r3,1
0:	j_s.d	[blink]
	sub	r0,r3,r0

#define SPECIAL_EARLY_END
.Learly_end:
	mov_s	r3,r0
	b_s	1b
#elif 0 /* Need more information about pipeline to assess if this is faster.  */
	mov.eq  r2,r6
	and	r2,r2,r5
	sub1	r2,r4,r2
	mov.eq  r1,r12
	bic.f   r1,r1,r2
	sub     r0,r3,r0
	add.pl  r0,r0,1
	btst.pl	r1,23
	add.eq	r0,r0,1
	btst.eq	r1,15
	j_s.d	[blink]
	add.eq	r0,r0,1
#else /* !__OPTIMIZE_SIZE__ */
	/* Need carry clear here.  */
	mov.eq	r2,r6
1:	bmsk	r1,r2,23
	breq	r1,r2,0f
	bmsk	r2,r1,15
	breq.d	r1,r2,0f
	add_s	r3,r3,1
	cmp	r2,0x100
	add_s	r3,r3,2
0:	j_s.d	[blink]
	sbc	r0,r3,r0

#define SPECIAL_EARLY_END
.Learly_end:
	sub_s.ne r1,r1,r1
	mov_s	r12,0
	bset	r12,r12,r7
	sub1	r2,r2,r12
	b.d	.Lend
	sub1.ne	r6,r6,r12
#endif /* !__OPTIMIZE_SIZE__ */
#endif /* ENDIAN */

#ifndef SPECIAL_EARLY_END
	.balign	4
.Learly_end:
	b.d	.Lend
	sub_s.ne r1,r1,r1
#endif /* !SPECIAL_EARLY_END */
ENDFUNC (strlen)
#endif /* __ARC601__ || !__ARC_BARREL_SHIFTER__*/

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
