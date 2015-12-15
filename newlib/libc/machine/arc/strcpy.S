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

#if defined (__ARC601__) || !defined (__ARC_BARREL_SHIFTER__)
/* If dst and src are 4 byte aligned, copy 8 bytes at a time.
   If the src is 4, but not 8 byte aligned, we first read 4 bytes to get
   it 8 byte aligned.  Thus, we can do a little read-ahead, without
   dereferencing a cache line that we should not touch.
   This version is a compromise between speed for the 601 pipeline and code
   size.  */

ENTRY (strcpy)
	or	r2,r0,r1
	bmsk.f	0,r2,1
	mov	r8,0x01010101
	bne.d	.Lcharloop
	mov_s	r10,r0
	ld_s	r3,[r1]
	bbit0.d	r1,2,.Loop_start
	ror	r12,r8
	sub	r2,r3,r8
	bic_s	r2,r2,r3
	and_s	r2,r2,r12
	brne_s	r2,0,.Lr3z
	mov	r4,r3
	sub_s	r1,r1,4
	.balign	4
.Loop:
	ld.a	r3,[r1,8]
	st.ab	r4,[r10,4]
.Loop_start:
	ld	r4,[r1,4]
	sub	r2,r3,r8
	bic_s	r2,r2,r3
	tst_s	r2,r12
	sub	r5,r4,r8
	bic	r5,r5,r4
	bne_s	.Lr3z
	and	r5,r5,r12
	breq.d	r5,0,.Loop
	st.ab	r3,[r10,4]

	;mov_s	r3,r4
	add_s	r1,r1,4

	.balign	4
.Lr3z:
.Lcharloop:
	ldb.ab	r3,[r1,1]
	brne.d	r3,0,.Lcharloop
	stb.ab	r3,[r10,1]
	j_s	[blink]
ENDFUNC (strcpy)
#endif /* __ARC601__ || !__ARC_BARREL_SHIFTER__ */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
