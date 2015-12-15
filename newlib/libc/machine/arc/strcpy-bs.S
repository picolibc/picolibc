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
   implementation of this function from newlib/libc/string/strcpy.c will be
   used.  */
#if !defined (__OPTIMIZE_SIZE__) && !defined (PREFER_SIZE_OVER_SPEED)

#include "asm.h"

#if (defined (__ARC700__) || defined (__ARCEM__) || defined (__ARCHS__)) \
    && defined (__ARC_BARREL_SHIFTER__)

/* If dst and src are 4 byte aligned, copy 8 bytes at a time.
   If the src is 4, but not 8 byte aligned, we first read 4 bytes to get
   it 8 byte aligned.  Thus, we can do a little read-ahead, without
   dereferencing a cache line that we should not touch.
   Note that short and long instructions have been scheduled to avoid
   branch stalls.
   The beq_s to r3z could be made unaligned & long to avoid a stall
   there, but the it is not likely to be taken often, and it
   would also be likey to cost an unaligned mispredict at the next call.  */

ENTRY (strcpy)
	or	r2,r0,r1
	bmsk_s	r2,r2,1
	brne.d	r2,0,charloop
	mov_s	r10,r0
	ld_s	r3,[r1,0]
	mov	r8,0x01010101
	bbit0.d	r1,2,loop_start
	ror	r12,r8
	sub	r2,r3,r8
	bic_s	r2,r2,r3
	tst_s	r2,r12
	bne_l	r3z
	mov_s	r4,r3
	.balign 4
loop:
	ld.a	r3,[r1,4]
	st.ab	r4,[r10,4]
loop_start:
	ld.a	r4,[r1,4]
	sub	r2,r3,r8
	bic_s	r2,r2,r3
	tst_l	r2,r12
	bne_l	r3z
	st.ab	r3,[r10,4]
	sub	r2,r4,r8
	bic	r2,r2,r4
	tst_l	r2,r12
	beq_l	loop
	mov_s	r3,r4
#ifdef __LITTLE_ENDIAN__
r3z:	bmsk.f	r1,r3,7
	lsr_s	r3,r3,8
#else
r3z:	lsr.f	r1,r3,24
	asl_s	r3,r3,8
#endif
	bne.d	r3z
	stb.ab	r1,[r10,1]
	j_s	[blink]

	.balign	4
charloop:
	ldb.ab	r3,[r1,1]


	brne.d	r3,0,charloop
	stb.ab	r3,[r10,1]
	j	[blink]
ENDFUNC (strcpy)
#endif /* (__ARC700__ || __ARCEM__ || __ARCHS__) && __ARC_BARREL_SHIFTER__ */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
