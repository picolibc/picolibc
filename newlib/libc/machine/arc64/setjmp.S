/*
   Copyright (c) 2024, Synopsys, Inc. All rights reserved.

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

#include <sys/asm.h>

/* ABI interface file
   these are the stack mappings for the registers
   as stored in the ABI for ARC */

ABIr14	= 0
ABIr15	= ABIr14 + REG_SZ
ABIr16	= ABIr15 + REG_SZ
ABIr17	= ABIr16 + REG_SZ
ABIr18	= ABIr17 + REG_SZ
ABIr19	= ABIr18 + REG_SZ
ABIr20	= ABIr19 + REG_SZ
ABIr21	= ABIr20 + REG_SZ
ABIr22	= ABIr21 + REG_SZ
ABIr23	= ABIr22 + REG_SZ
ABIr24	= ABIr23 + REG_SZ
ABIr25	= ABIr24 + REG_SZ
ABIr26	= ABIr25 + REG_SZ
ABIr27	= ABIr26 + REG_SZ
ABIr28	= ABIr27 + REG_SZ
ABIr29	= ABIr28 + REG_SZ
ABIr30	= ABIr29 + REG_SZ
ABIr31	= ABIr30 + REG_SZ


ENTRY (setjmp)
	REG_ST	r14, [r0, ABIr14]
	REG_ST	r15, [r0, ABIr15]
	REG_ST	r16, [r0, ABIr16]
	REG_ST	r17, [r0, ABIr17]
	REG_ST	r18, [r0, ABIr18]
	REG_ST	r19, [r0, ABIr19]
	REG_ST	r20, [r0, ABIr20]
	REG_ST	r21, [r0, ABIr21]
	REG_ST	r22, [r0, ABIr22]
	REG_ST	r23, [r0, ABIr23]
	REG_ST	r24, [r0, ABIr24]
	REG_ST	r25, [r0, ABIr25]
	REG_ST	r26, [r0, ABIr26]
	REG_ST	r27, [r0, ABIr27]
	REG_ST	r28, [r0, ABIr28]
	REG_ST	r29, [r0, ABIr29]
	REG_ST	r30, [r0, ABIr30]
	REG_ST	blink, [r0, ABIr31]

	j.d	[blink]
	mov	r0,0
	.size	setjmp,.-setjmp

ENTRY (longjmp)
	; load registers
	REG_LD	r14, [r0, ABIr14]
	REG_LD	r15, [r0, ABIr15]
	REG_LD	r16, [r0, ABIr16]
	REG_LD	r17, [r0, ABIr17]
	REG_LD	r18, [r0, ABIr18]
	REG_LD	r19, [r0, ABIr19]
	REG_LD	r20, [r0, ABIr20]
	REG_LD	r21, [r0, ABIr21]
	REG_LD	r22, [r0, ABIr22]
	REG_LD	r23, [r0, ABIr23]
	REG_LD	r24, [r0, ABIr24]
	REG_LD	r25, [r0, ABIr25]
	REG_LD	r26, [r0, ABIr26]
	REG_LD	r27, [r0, ABIr27]
	REG_LD	r28, [r0, ABIr28]
	REG_LD	r29, [r0, ABIr29]
	REG_LD	r30, [r0, ABIr30]
	REG_LD	blink, [r0, ABIr31]

	mov.f	r1, r1			; to avoid return 0 from longjmp
	mov.z	r1, 1
	j.d	[blink]
	mov	r0,r1
	.size	longjmp,.-longjmp
