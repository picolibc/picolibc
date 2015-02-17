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
	.global __aeabi_memset
	.type	__aeabi_memset, %function
	ASM_ALIAS __aeabi_memset4 __aeabi_memset
	ASM_ALIAS __aeabi_memset8 __aeabi_memset
__aeabi_memset:
	.cfi_startproc
	push	{r4, r5, r6}
	lsls	r4, r0, #30
	beq	.L14
	subs	r4, r1, #1
	cmp	r1, #0
	beq	.L16
	uxtb	r5, r2
	mov	r3, r0
	b	.L4
.L6:
	subs	r1, r4, #1
	cbz	r4, .L16
	mov	r4, r1
.L4:
	strb	r5, [r3], #1
	lsls	r1, r3, #30
	bne	.L6
.L2:
	cmp	r4, #3
	bls	.L11
	uxtb	r5, r2
	orr	r5, r5, r5, lsl #8
	cmp	r4, #15
	orr	r5, r5, r5, lsl #16
	bls	.L9
	add	r1, r3, #16
	mov	r6, r4
.L10:
	subs	r6, r6, #16
	cmp	r6, #15
	str	r5, [r1, #-16]
	str	r5, [r1, #-12]
	str	r5, [r1, #-8]
	str	r5, [r1, #-4]
	add	r1, r1, #16
	bhi	.L10
	sub	r1, r4, #16
	bic	r1, r1, #15
	and	r4, r4, #15
	adds	r1, r1, #16
	cmp	r4, #3
	add	r3, r3, r1
	bls	.L11
.L9:
	mov	r6, r3
	mov	r1, r4
.L12:
	subs	r1, r1, #4
	cmp	r1, #3
	str	r5, [r6], #4
	bhi	.L12
	subs	r1, r4, #4
	bic	r1, r1, #3
	adds	r1, r1, #4
	add	r3, r3, r1
	and	r4, r4, #3
.L11:
	cbz	r4, .L16
	uxtb	r2, r2
	add	r4, r4, r3
.L13:
	strb	r2, [r3], #1
	cmp	r3, r4
	bne	.L13
.L16:
	pop	{r4, r5, r6}
	bx	lr
.L14:
	mov	r4, r1
	mov	r3, r0
	b	.L2
	.cfi_endproc
	.size __aeabi_memset, . - __aeabi_memset
