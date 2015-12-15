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

/* ARC HS has it's own implementation of memset, yet we want this function
   still to be compiled under "__dummy_memset" disguise, because strncpy
   function uses __strncpy_bzero as a second entry point into memset.  Would be
   better to add __strncpy_bzero label to memset for ARC HS though, and even
   better would be to avoid a second entry point into function.  ARC HS always
   has barrel-shifter, so this implementation will be always used for this
   purpose.  */
#if !defined (__ARC601__) && defined (__ARC_BARREL_SHIFTER__)

/* To deal with alignment/loop issues, SMALL must be at least 2.  */
#define SMALL	7

	.global	__strncpy_bzero
	.hidden __strncpy_bzero
/* __strncpy_bzero provides the following interface to strncpy:
   r0: return value
   r2: zeroing length
   r3: zeroing start address
   No attempt is made here for __strncpy_memset to speed up aligned
   cases, because the copying of a string presumably leaves start address
   and length alignment for the zeroing randomly distributed.  */

#ifdef __ARCHS__
ENTRY (__dummy_memset)
#else
ENTRY (memset)
#endif
#if !defined (__ARC700__) && !defined (__ARCEM__)
#undef SMALL
#define SMALL 8 /* Even faster if aligned.  */
	brls.d	r2,SMALL,.Ltiny
#endif
	mov_s	r3,r0
	or	r12,r0,r2
	bmsk.f	r12,r12,1
	extb_s	r1,r1
	asl	r12,r1,8
	beq.d	.Laligned
	or_s	r1,r1,r12
#if defined (__ARC700__) || defined (__ARCEM__)
	brls	r2,SMALL,.Ltiny
#endif
.Lnot_tiny:
	add_s	r12,r2,r0
	stb	r1,[r12,-1]
	bclr_l	r12,r12,0
	stw	r1,[r12,-2]
	bmsk.f	r12,r3,1
	add_s	r2,r2,r12
	sub.ne	r2,r2,4
	stb.ab	r1,[r3,1]
	bclr_s	r3,r3,0
	stw.ab	r1,[r3,2]
	bclr_s	r3,r3,1
.Laligned:	; This code address should be aligned for speed.
#if defined (__ARC700__) || defined (__ARCEM__)
	asl	r12,r1,16
	lsr.f	lp_count,r2,2
	or_s	r1,r1,r12
	lpne	.Loop_end
	st.ab	r1,[r3,4]
.Loop_end:
	j_s	[blink]
#else /* !__ARC700 */
	lsr.f	lp_count,r2,3
	asl	r12,r1,16
	or_s	r1,r1,r12
	lpne	.Loop_end
	st.ab	r1,[r3,4]
	st.ab	r1,[r3,4]
.Loop_end:
	jcc	[blink]
	j_s.d	[blink]
	st_s	r1,[r3]
#endif /* !__ARC700 */

#if defined (__ARC700__) || defined (__ARCEM__)
	.balign	4
__strncpy_bzero:
	brhi.d	r2,17,.Lnot_tiny
	mov_l	r1,0
.Ltiny:
	mov.f	lp_count,r2
	lpne	.Ltiny_end
	stb.ab	r1,[r3,1]
.Ltiny_end:
	j_s	[blink]
#else /* !__ARC700__ */
#if SMALL > 8
FIXME
#endif
	.balign	4
__strncpy_bzero:
	brhi.d	r2,8,.Lnot_tiny
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
#endif /* !__ARC700 */
#ifdef __ARCHS__
ENDFUNC (__dummy_memset)
#else
ENDFUNC (memset)
#endif
#endif /* !__ARC601__ && __ARC_BARREL_SHIFTER__ */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
