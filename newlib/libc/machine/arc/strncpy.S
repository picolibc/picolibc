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
   implementation of this function from newlib/libc/string/strncpy.c will be
   used.  */
#if !defined (__OPTIMIZE_SIZE__) && !defined (PREFER_SIZE_OVER_SPEED)

#include "asm.h"

/* If dst and src are 4 byte aligned, copy 8 bytes at a time.
   If the src is 4, but not 8 byte aligned, we first read 4 bytes to get
   it 8 byte aligned.  Thus, we can do a little read-ahead, without
   dereferencing a cache line that we should not touch.  */

#if defined (__ARC601__) || !defined (__ARC_BARREL_SHIFTER__)

#define BRand(a,b,l)	and	a,a,b ` brne_s a,0,l

ENTRY (strncpy)
	cmp_s	r2,8
	or	r12,r0,r1
	bmsk.cc.f r12,r12,1
	brne.d	r12,0,.Lbytewise
	mov_s	r10,r0
	ld_s	r3,[r1,0]
	mov	r8,0x01010101
	add	r6,r0,r2
	sub	r6,r6,8
	bbit0.d	r1,2,.Loop_start
	ror	r11,r8
	sub	r12,r3,r8
	bic_l	r12,r12,r3
	BRand	(r12,r11,.Lr3z)
	mov_s	r4,r3
	ld.a	r3,[r1,4]
	st.ab	r4,[r10,4]
	.balign 4
.Loop_start:
	brhs	r10,r6,.Loop_end
1:
	ld.a	r4,[r1,4]
	sub	r12,r3,r8
	bic_s	r12,r12,r3
	BRand	(r12,r11,.Lr3z2)
	st.ab	r3,[r10,8]
	sub	r12,r4,r8
	bic	r12,r12,r4
	BRand	(r12,r11,.Lr4z)
	ld.a	r3,[r1,4]
	brlo.d	r10,r6,1b
	st	r4,[r10,-4]
.Loop_end:
	add	r6,r6,4
	brhs	r10,r6,.Lastword
	sub	r12,r3,r8
	bic_s	r12,r12,r3
	BRand	(r12,r11,.Lr3z)
	add_s	r1,r1,4
	st.ab	r3,[r10,4]
.Lastword:
	sub_s	r2,r2,1
	b.d .Lstart_charloop
	bmsk.f	r2,r2,1

	.balign 4
	nop_s
.Lr3z2:	sub_s	r1,r1,4
.Lr4z:
.Lr3z:
	.balign	4
.Lr3z_loop:
	ldb.ab	r3,[r1,1]
        brne.d	r3,0,.Lr3z_loop
        stb.ab  r3,[r10,1]

.Lzero_rest:
	; __strncpy_bzero requires:
        ; return value in r0
        ; zeroing length in r2
        ; zeroing start address in r3
	mov_s	r3,r10
	add_s	r2,r2,r0
        b.d	__strncpy_bzero
	sub_s	r2,r2,r3

	.balign	4
.Lbytewise:
	sub.f	r2,r2,1
	jcs	[blink]
.Lstart_charloop:
	mov_s	r3,r10
.Lcharloop:
	ldb.ab	r12,[r1,1]
	beq.d	.Last_byte
	sub.f	r2,r2,1
	brne.d	r12,0,.Lcharloop
	stb.ab	r12,[r3,1]
        b.d	__strncpy_bzero
	stb.ab	r12,[r3,1]
.Last_byte:
	j_s.d	[blink]
	stb_s	r12,[r3]
ENDFUNC (strncpy)
#endif /* __ARC601__ || !__ARC_BARREL_SHIFTER__ */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
