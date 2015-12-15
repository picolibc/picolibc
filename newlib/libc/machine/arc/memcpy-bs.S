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
   implementation of this function from newlib/libc/string/memcpy.c will be
   used.  */
#if !defined (__OPTIMIZE_SIZE__) && !defined (PREFER_SIZE_OVER_SPEED)

#include "asm.h"

#if !defined (__ARC601__) && !defined (__ARCHS__) \
    && defined (__ARC_BARREL_SHIFTER__)

/* Mostly optimized for ARC700, but not bad for ARC600 either.  */
/* This memcpy implementation does not support objects of 1GB or larger -
   the check for alignment does not work then.  */
/* We assume that most sources and destinations are aligned, and
   that also lengths are mostly a multiple of four, although to a lesser
   extent.  */
ENTRY (memcpy)
	or	r3,r0,r1
	asl_s	r3,r3,30
	mov_s	r5,r0
	brls.d	r2,r3,.Lcopy_bytewise
	sub.f	r3,r2,1
	ld_s	r12,[r1,0]
	asr.f	lp_count,r3,3
	bbit0.d	r3,2,.Lnox4
	bmsk_s	r2,r2,1
	st.ab	r12,[r5,4]
	ld.a	r12,[r1,4]
.Lnox4:
	lppnz	.Lendloop
	ld_s	r3,[r1,4]
	st.ab	r12,[r5,4]
	ld.a	r12,[r1,8]
	st.ab	r3,[r5,4]
.Lendloop:
	breq_l	r2,0,.Last_store
	ld	r3,[r5,0]
#ifdef __LITTLE_ENDIAN__
	add3	r2,-1,r2
	; uses long immediate
	xor_s	r12,r12,r3
	bmsk	r12,r12,r2
        xor_s	r12,r12,r3
#else /* BIG ENDIAN */
	sub3	r2,31,r2
	; uses long immediate
        xor_s	r3,r3,r12
        bmsk	r3,r3,r2
        xor_s	r12,r12,r3
#endif /* ENDIAN */
.Last_store:
	j_s.d	[blink]
	st	r12,[r5,0]

	.balign	4
.Lcopy_bytewise:
	jcs	[blink]
	ldb_s	r12,[r1,0]
	lsr.f	lp_count,r3
	bcc_s	.Lnox1
	stb.ab	r12,[r5,1]
	ldb.a	r12,[r1,1]
.Lnox1:
	lppnz	.Lendbloop
	ldb_s	r3,[r1,1]
	stb.ab	r12,[r5,1]
	ldb.a	r12,[r1,2]
	stb.ab	r3,[r5,1]
.Lendbloop:
	j_s.d	[blink]
	stb	r12,[r5,0]
ENDFUNC (memcpy)
#endif /* !__ARC601__ && !__ARCHS__ && __ARC_BARREL_SHIFTER__ */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
