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

#ifdef __ARCHS__

#define USE_PREFETCH

#ifdef USE_PREFETCH
#define PREWRITE(A,B)	prefetchw [(A),(B)]
#else
#define PREWRITE(A,B)	prealloc [(A),(B)]
#endif

ENTRY (memset)
	prefetchw [r0]		; Prefetch the write location
	mov.f	0, r2
; if size is zero
	jz.d	[blink]
	mov	r3, r0		; don't clobber ret val

; if length < 8
	brls.d.nt	r2, 8, .Lsmallchunk
	mov.f	lp_count,r2

	and.f	r4, r0, 0x03
	rsub	lp_count, r4, 4
	lpnz	@.Laligndestination
	; LOOP BEGIN
	stb.ab	r1, [r3,1]
	sub	r2, r2, 1
.Laligndestination:

; Destination is aligned
	and	r1, r1, 0xFF
	asl	r4, r1, 8
	or	r4, r4, r1
	asl	r5, r4, 16
	or	r5, r5, r4
	mov	r4, r5

	sub3	lp_count, r2, 8
	cmp     r2, 64
	bmsk.hi	r2, r2, 5
	mov.ls	lp_count, 0
	add3.hi	r2, r2, 8

; Convert len to Dwords, unfold x8
	lsr.f	lp_count, lp_count, 6
	lpnz	@.Lset64bytes
	; LOOP START
	PREWRITE (r3, 64)	;Prefetch the next write location
#ifdef __ARC_LL64__
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
#else
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
#endif
.Lset64bytes:

	lsr.f	lp_count, r2, 5 ;Last remaining  max 124 bytes
	lpnz	.Lset32bytes
	; LOOP START
	prefetchw [r3, 32]	;Prefetch the next write location
#ifdef __ARC_LL64__
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
#else
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
#endif
.Lset32bytes:

	and.f	lp_count, r2, 0x1F ;Last remaining 31 bytes
.Lsmallchunk:
	lpnz	.Lcopy3bytes
	; LOOP START
	stb.ab	r1, [r3, 1]
.Lcopy3bytes:

	j	[blink]

ENDFUNC (memset)
#endif /* __ARCHS__ */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
