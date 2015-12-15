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
   implementation of this function from newlib/libc/string/strcmp.c will be
   used.  */
#if !defined (__OPTIMIZE_SIZE__) && !defined (PREFER_SIZE_OVER_SPEED)

#include "asm.h"

#ifdef __ARCHS__
ENTRY (strcmp)
	or	r2, r0, r1
	bmsk_s	r2, r2, 1
	brne	r2, 0, @.Lcharloop

; s1 and s2 are word aligned
	ld.ab	r2, [r0, 4]

	mov_s	r12, 0x01010101
	ror	r11, r12
	.align  4
.LwordLoop:
	ld.ab	r3, [r1, 4]
	; Detect NULL char in str1
	sub	r4, r2, r12
	ld.ab	r5, [r0, 4]
	bic	r4, r4, r2
	and	r4, r4, r11
	brne.d.nt	r4, 0, .LfoundNULL
	; Check if the read locations are the same
	cmp	r2, r3
	beq.d	.LwordLoop
	mov.eq	r2, r5

	; A match is found, spot it out
#ifdef __LITTLE_ENDIAN__
	swape	r3, r3
	mov_s	r0, 1
	swape	r2, r2
#else
	mov_s	r0, 1
#endif
	cmp_s	r2, r3
	j_s.d	[blink]
	bset.lo	r0, r0, 31

	.align 4
.LfoundNULL:
#ifdef __BIG_ENDIAN__
	swape	r4, r4
	swape	r2, r2
	swape	r3, r3
#endif
	; Find null byte
	ffs	r0, r4
	bmsk	r2, r2, r0
	bmsk	r3, r3, r0
	swape	r2, r2
	swape	r3, r3
	; make the return value
	sub.f	r0, r2, r3
	mov.hi	r0, 1
	j_s.d	[blink]
	bset.lo	r0, r0, 31

	.align 4
.Lcharloop:
	ldb.ab	r2, [r0, 1]
	ldb.ab	r3, [r1, 1]
	nop
	breq	r2, 0, .Lcmpend
	breq	r2, r3, .Lcharloop

	.align 4
.Lcmpend:
	j_s.d	[blink]
	sub	r0, r2, r3
ENDFUNC (strcmp)
#endif /* __ARCHS__ */

#endif /* !__OPTIMIZE_SIZE__ && !PREFER_SIZE_OVER_SPEED */
