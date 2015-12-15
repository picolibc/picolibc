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
   implementation of this function from newlib/libc/string/memcmp.c will be
   used.  */
#if !defined (__OPTIMIZE_SIZE__) && !defined (PREFER_SIZE_OVER_SPEED)

#include "asm.h"

#if defined (__ARC601__) || !defined (__ARC_NORM__) \
    || !defined (__ARC_BARREL_SHIFTER__)

/* Addresses are unsigned, and at 0 is the vector table, so it's OK to assume
   that we can subtract 8 from a source end address without underflow.  */

ENTRY (memcmp)
	or	r12,r0,r1
	tst	r12,3
	breq	r2,0,.Lnil
	add_s	r3,r0,r2

/* This algorithm for big endian targets sometimes works incorrectly
   when sources are aligned. To be precise the last step is omitted.
   Just use a simple bytewise variant until the algorithm is reviewed
   and fixed.  */

#ifdef __LITTLE_ENDIAN__
	bne_s	.Lbytewise
#else /* BIG ENDIAN */
	b_s	.Lbytewise
#endif /* ENDIAN */
	sub	r6,r3,8
	ld	r4,[r0,0]
	ld	r5,[r1,0]
2:
	brhs	r0,r6,.Loop_end
	ld_s	r3,[r0,4]
	ld_s	r12,[r1,4]
	brne	r4,r5,.Leven
	ld.a	r4,[r0,8]
	breq.d	r3,r12,2b
	ld.a	r5,[r1,8]
#ifdef __LITTLE_ENDIAN__
	mov_s	r4,r3
	b.d	.Lodd
	mov_s	r5,r12
#else /* BIG ENDIAN */
	cmp_s	r3,r12
	j_s.d	[blink]
	rrc	r0,2
#endif /* ENDIAN */

	.balign	4
.Loop_end:
	sub	r3,r0,r6
	brhs	r3,4,.Last_cmp
	brne	r4,r5,.Leven
	ld	r4,[r0,4]
	ld	r5,[r1,4]
#ifdef __LITTLE_ENDIAN__
	.balign	4
.Last_cmp:
	mov_l	r0,24
	add3	r2,r0,r2
	xor	r0,r4,r5
	b.d	.Leven_cmp
	bset	r0,r0,r2
.Lodd:
.Leven:
	xor	r0,r4,r5
.Leven_cmp:
	mov_s	r1,0x80808080
	; uses long immediate
	sub_s	r12,r0,1
	bic_s	r0,r0,r12
	sub	r0,r1,r0
	xor_s	r0,r0,r1
	and	r1,r5,r0
	and	r0,r4,r0
#else /* BIG ENDIAN */
.Last_cmp:
	mov_s	r3,0
	sub3	r2,r3,r2
	sub_s	r3,r3,1
	bclr	r3,r3,r2
	add_l	r3,r3,1
	and	r0,r4,r3
	and	r1,r5,r3
.Leven:
#endif /* ENDIAN */
	xor.f	0,r0,r1
	sub_s	r0,r0,r1
	j_s.d	[blink]
	mov.mi	r0,r1
	.balign	4
.Lbytewise:
	ldb	r4,[r0,0]
	ldb	r5,[r1,0]
	sub	r6,r3,2
3:
	brhs	r0,r6,.Lbyte_end
	ldb_s	r3,[r0,1]
	ldb_s	r12,[r1,1]
	brne	r4,r5,.Lbyte_even
	ldb.a	r4,[r0,2]
	breq.d	r3,r12,3b
	ldb.a	r5,[r1,2]
.Lbyte_odd:
	j_s.d	[blink]
	sub	r0,r3,r12
	.balign	4
.Lbyte_end:
	bbit1	r2,0,.Lbyte_even
	brne	r4,r5,.Lbyte_even
	ldb	r4,[r0,1]
	ldb	r5,[r1,1]
.Lbyte_even:
	j_s.d	[blink]
	sub	r0,r4,r5
.Lnil:
	j_s.d	[blink]
	mov_s	r0,0
ENDFUNC (memcmp)
#endif /* __ARC601__ || !__ARC_NORM__ || !__ARC_BARREL_SHIFTER__ */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
