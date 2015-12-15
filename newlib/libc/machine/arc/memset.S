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
   implementation of this function from newlib/libc/string/memset.c will be
   used.  */
#if !defined (__OPTIMIZE_SIZE__) && !defined (PREFER_SIZE_OVER_SPEED)

#include "asm.h"

#if defined (__ARC601__) \
    || (!defined (__ARC_BARREL_SHIFTER__) && !defined (__ARCHS__))

/* To deal with alignment/loop issues, SMALL must be at least 2.  */
#define SMALL 8 /* Even faster if aligned.  */

	.global	__strncpy_bzero
	.hidden __strncpy_bzero
/* __strncpy_bzero provides the following interface to strncpy:
   r0: return value
   r2: zeroing length
   r3: zeroing start address
   No attempt is made here for __strncpy_memset to speed up aligned
   cases, because the copying of a string presumably leaves start address
   and length alignment for the zeroing randomly distributed.  */

ENTRY (memset)
	brls.d	r2,SMALL,.Ltiny
	mov_s	r3,r0
	or	r12,r0,r2
	bmsk.f	r12,r12,1
	breq_s	r1,0,.Lbzero
	mov	r4,0
	stb.a	r1,[sp,-4]
	stb	r1,[sp,1]
	stb	r1,[sp,2]
	stb	r1,[sp,3]
	ld.ab	r1,[sp,4]
.Lbzero:
	beq.d	.Laligned
.Lbzero2:
	add	r6,r2,r3
.Lnot_tiny:
	stb	r1,[r6,-1]
	bclr	r12,r6,0
	stw	r1,[r12,-2]
	stb.ab	r1,[r3,1]
	bclr_s	r3,r3,0
	stw.ab	r1,[r3,2]
	bclr_s	r3,r3,1
.Laligned:	; This code address should be aligned for speed.
	sub	r6,r6,8
	brlo.d	r6,r3,.Loop_end
	sub	r6,r6,8
3:
	st_l	r1,[r3,4]
	brhs.d	r6,r3,3b
	st.ab	r1,[r3,8]
.Loop_end:
	bic	r12,r6,3
	j_s.d	[blink]
	st_s	r1,[r12,12]
	.balign	4
__strncpy_bzero:
	brhi.d	r2,8,.Lbzero2
	mov_s	r1,0
.Ltiny:
	sub_s	r2,r2,11
	sub1	r12,pcl,r2
	j_s	[r12]
	stb_s	r1,[r3,7]
	stb_s	r1,[r3,6]
	stb_s	r1,[r3,5]
	stb_s	r1,[r3,4]
	stb_s	r1,[r3,3]
	stb_s	r1,[r3,2]
	stb_s	r1,[r3,1]
	stb_s	r1,[r3]
	j_s	[blink]
ENDFUNC (memset)
#endif /* __ARC601__ || (!__ARC_BARREL_SHIFTER__ && !__ARCHS__)  */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
