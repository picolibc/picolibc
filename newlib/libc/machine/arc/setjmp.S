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

/* ABI interface file
   these are the stack mappings for the registers
   as stored in the ABI for ARC */

       .file "setjmp.S"

ABIr13	= 0
ABIr14	= ABIr13 + 4
ABIr15	= ABIr14 + 4
ABIr16	= ABIr15 + 4
ABIr17	= ABIr16 + 4
ABIr18	= ABIr17 + 4
ABIr19	= ABIr18 + 4
ABIr20	= ABIr19 + 4
ABIr21	= ABIr20 + 4
ABIr22	= ABIr21 + 4
ABIr23	= ABIr22 + 4
ABIr24	= ABIr23 + 4
ABIr25	= ABIr24 + 4
ABIr26	= ABIr25 + 4
ABIr27	= ABIr26 + 4
ABIr28	= ABIr27 + 4
ABIr29	= ABIr28 + 4
ABIr30	= ABIr29 + 4
ABIr31	= ABIr30 + 4
ABIlpc	= ABIr31 + 4
ABIlps	= ABIlpc + 4
ABIlpe	= ABIlps + 4

ABIflg	= ABIlpe + 4
ABImlo	= ABIflg + 4
ABImhi	= ABImlo + 4

	.text
	.align 4
	.global setjmp
	.type setjmp,@function
setjmp:
	st	r13, [r0, ABIr13]
	st	r14, [r0, ABIr14]
	st	r15, [r0, ABIr15]
	st	r16, [r0, ABIr16]
	st	r17, [r0, ABIr17]
	st	r18, [r0, ABIr18]
	st	r19, [r0, ABIr19]
	st	r20, [r0, ABIr20]
	st	r21, [r0, ABIr21]
	st	r22, [r0, ABIr22]
	st	r23, [r0, ABIr23]
	st	r24, [r0, ABIr24]
	st	r25, [r0, ABIr25]
	st	r26, [r0, ABIr26]
	st	r27, [r0, ABIr27]
	st	r28, [r0, ABIr28]
	st	r29, [r0, ABIr29]
	st	r30, [r0, ABIr30]
	st	blink, [r0, ABIr31]
	st	lp_count, [r0, ABIlpc]

	lr	r2, [lp_start]
	lr	r3, [lp_end]
	st	r2, [r0, ABIlps]
	st	r3, [r0, ABIlpe]

#if (!defined (__ARC700__) && !defined (__ARCEM__) && !defined (__ARCHS__))
; Till the configure changes are decided, and implemented, the code working on
; mlo/mhi and using mul64 should be disabled.
; 	st	mlo, [r0, ABImlo]
; 	st	mhi, [r0, ABImhi]
	lr	r2, [status32]
	st	r2, [r0, ABIflg]
#endif

	j.d	[blink]
	mov	r0,0
.Lfe1:
	.size	setjmp,.Lfe1-setjmp

	.align	4
	.global longjmp
	.type longjmp,@function
longjmp:

	; load registers
	ld	r13, [r0, ABIr13]
	ld	r14, [r0, ABIr14]
	ld	r15, [r0, ABIr15]
	ld	r16, [r0, ABIr16]
	ld	r17, [r0, ABIr17]
	ld	r18, [r0, ABIr18]
	ld	r19, [r0, ABIr19]
	ld	r20, [r0, ABIr20]
	ld	r21, [r0, ABIr21]
	ld	r22, [r0, ABIr22]
	ld	r23, [r0, ABIr23]
	ld	r24, [r0, ABIr24]
	ld	r25, [r0, ABIr25]
	ld	r26, [r0, ABIr26]
	ld	r27, [r0, ABIr27]
	ld	r28, [r0, ABIr28]

	ld	r3, [r0, ABIr29]
	mov	r29, r3

	ld	r3, [r0, ABIr30]
	mov	r30, r3

	ld	blink, [r0, ABIr31]

	ld	r3,  [r0, ABIlpc]
	mov	lp_count, r3

	ld	r2, [r0, ABIlps]
	ld	r3, [r0, ABIlpe]
	sr	r2, [lp_start]
	sr	r3, [lp_end]

#if (!defined (__ARC700__) && !defined (__ARCEM__) && !defined (__ARCHS__))
	ld	r2, [r0, ABImlo]
	ld	r3, [r0, ABImhi]
; We do not support restoring of mulhi and mlo registers, yet.

;	mulu64	0,r2,1			; restores mlo
;	mov	0,mlo			; force multiply to finish
;	sr	r3, [mulhi]
	ld	r2, [r0, ABIflg]
	flag	r2			; restore "status32" register
#endif

	mov.f	r1, r1			; to avoid return 0 from longjmp
	mov.eq	r1, 1
	j.d	[blink]
	mov	r0,r1
.Lfe2:
	.size	longjmp,.Lfe2-longjmp
