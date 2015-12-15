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

#if !defined (__ARC601__) && defined (__ARC_NORM__) \
    && defined (__ARC_BARREL_SHIFTER__)

#ifdef __LITTLE_ENDIAN__
#define WORD2 r2
#define SHIFT r3
#else /* BIG ENDIAN */
#define WORD2 r3
#define SHIFT r2
#endif

ENTRY (memcmp)
	or	r12,r0,r1
	asl_s	r12,r12,30
#if defined (__ARC700__) || defined (__ARCEM__) || defined (__ARCHS__)
	sub_l	r3,r2,1
	brls	r2,r12,.Lbytewise
#else
	brls.d	r2,r12,.Lbytewise
	sub_s	r3,r2,1
#endif
	ld	r4,[r0,0]
	ld	r5,[r1,0]
	lsr.f	lp_count,r3,3
#ifdef __ARCEM__
	/* A branch can't be the last instruction in a zero overhead loop.
	   So we move the branch to the start of the loop, duplicate it
	   after the end, and set up r12 so that the branch isn't taken
	   initially.  */
	mov_s	r12,WORD2
	lpne	.Loop_end
	brne	WORD2,r12,.Lodd
	ld	WORD2,[r0,4]
#else
	lpne	.Loop_end
	ld_s	WORD2,[r0,4]
#endif
	ld_s	r12,[r1,4]
	brne	r4,r5,.Leven
	ld.a	r4,[r0,8]
	ld.a	r5,[r1,8]
#ifdef __ARCEM__
.Loop_end:
	brne	WORD2,r12,.Lodd
#else
	brne	WORD2,r12,.Lodd
#ifdef __ARCHS__
	nop
#endif
.Loop_end:
#endif
	asl_s	SHIFT,SHIFT,3
	bcc_s	.Last_cmp
	brne	r4,r5,.Leven
	ld	r4,[r0,4]
	ld	r5,[r1,4]
#ifdef __LITTLE_ENDIAN__
#if defined (__ARC700__) || defined (__ARCEM__) || defined (__ARCHS__)
	nop_s
	; one more load latency cycle
.Last_cmp:
	xor	r0,r4,r5
	bset	r0,r0,SHIFT
	sub_s	r1,r0,1
	bic_s	r1,r1,r0
	norm	r1,r1
	b.d	.Leven_cmp
	and	r1,r1,24
.Leven:
	xor	r0,r4,r5
	sub_s	r1,r0,1
	bic_s	r1,r1,r0
	norm	r1,r1
	; slow track insn
	and	r1,r1,24
.Leven_cmp:
	asl	r2,r4,r1
	asl	r12,r5,r1
	lsr_s	r2,r2,1
	lsr_s	r12,r12,1
	j_s.d	[blink]
	sub	r0,r2,r12
	.balign	4
.Lodd:
	xor	r0,WORD2,r12
	sub_s	r1,r0,1
	bic_s	r1,r1,r0
	norm	r1,r1
	; slow track insn
	and	r1,r1,24
	asl_s	r2,r2,r1
	asl_s	r12,r12,r1
	lsr_s	r2,r2,1
	lsr_s	r12,r12,1
	j_s.d	[blink]
	sub	r0,r2,r12
#else /* !__ARC700__ */
	.balign	4
.Last_cmp:
	xor	r0,r4,r5
	b.d	.Leven_cmp
	bset	r0,r0,SHIFT
.Lodd:
	mov_s	r4,WORD2
	mov_s	r5,r12
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
	xor.f	0,r0,r1
	sub_s	r0,r0,r1
	j_s.d	[blink]
	mov.mi	r0,r1
#endif /* !__ARC700__ */
#else /* BIG ENDIAN */
.Last_cmp:
	neg_s	SHIFT,SHIFT
	lsr	r4,r4,SHIFT
	lsr	r5,r5,SHIFT
	; slow track insn
.Leven:
	sub.f	r0,r4,r5
	mov.ne	r0,1
	j_s.d	[blink]
	bset.cs	r0,r0,31
.Lodd:
	cmp_s	WORD2,r12
#if defined (__ARC700__) || defined (__ARCEM__) || defined (__ARCHS__)
	mov_s	r0,1
	j_s.d	[blink]
	bset.cs	r0,r0,31
#else
	j_s.d	[blink]
	rrc	r0,2
#endif /* __ARC700__ || __ARCEM__ || __ARCHS__ */
#endif /* ENDIAN */
	.balign	4
.Lbytewise:
	breq	r2,0,.Lnil
	ldb	r4,[r0,0]
	ldb	r5,[r1,0]
	lsr.f	lp_count,r3
#ifdef __ARCEM__
	mov	r12,r3
	lpne	.Lbyte_end
	brne	r3,r12,.Lbyte_odd
#else
	lpne	.Lbyte_end
#endif
	ldb_s	r3,[r0,1]
	ldb_l	r12,[r1,1]
	brne	r4,r5,.Lbyte_even
	ldb.a	r4,[r0,2]
	ldb.a	r5,[r1,2]
#ifdef __ARCEM__
.Lbyte_end:
	brne	r3,r12,.Lbyte_odd
#else
	brne	r3,r12,.Lbyte_odd
#ifdef __ARCHS__
	nop
#endif
.Lbyte_end:
#endif
	bcc_l	.Lbyte_even
	brne	r4,r5,.Lbyte_even
	ldb_s	r3,[r0,1]
	ldb_s	r12,[r1,1]
.Lbyte_odd:
	j_s.d	[blink]
	sub	r0,r3,r12
.Lbyte_even:
	j_s.d	[blink]
	sub	r0,r4,r5
.Lnil:
	j_s.d	[blink]
	mov_l	r0,0
ENDFUNC (memcmp)
#endif /* !__ARC601__ && __ARC_NORM__ && __ARC_BARREL_SHIFTER__ */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
