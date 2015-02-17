/*
 * Copyright (c) 2015 ARM Ltd
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the company may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

	.thumb
	.syntax unified
	.global __aeabi_memmove
	.type	__aeabi_memmove, %function
	ASM_ALIAS __aeabi_memmove4 __aeabi_memmove
	ASM_ALIAS __aeabi_memmove8 __aeabi_memmove
__aeabi_memmove:
	.cfi_startproc
	push	{r4, lr}
	cmp	r0, r1
	bls	.L9
	adds	r3, r1, r2
	cmp	r0, r3
	bcc	.L3
.L9:
	movs	r3, #0
	b	.L4
.L3:
	subs	r3, r3, r2
.L5:
	subs	r2, r2, #1
	bcc	.L10
	ldrb	r1, [r3, r2]
	strb	r1, [r0, r2]
	b	.L5
.L4:
	cmp	r3, r2
	beq	.L10
	ldrb	r4, [r1, r3]
	strb	r4, [r0, r3]
	adds	r3, r3, #1
	b	.L4
.L10:
	pop	{r4, pc}
	.cfi_endproc
	.size __aeabi_memmove, . - __aeabi_memmove
