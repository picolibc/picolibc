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

	.arm
	.syntax divided
	.global __aeabi_memset
	.type	__aeabi_memset, %function
	ASM_ALIAS __aeabi_memset4 __aeabi_memset
	ASM_ALIAS __aeabi_memset8 __aeabi_memset
__aeabi_memset:
	.cfi_startproc
	tst	r0, #3
	stmfd	sp!, {r4, r5, lr}
	beq	.L14
	cmp	r1, #0
	sub	r1, r1, #1
	ldmeqfd	sp!, {r4, r5, pc}
	uxtb	lr, r2
	mov	r3, r0
	b	.L4
.L6:
	cmp	r1, #0
	mov	r1, ip
	ldmeqfd	sp!, {r4, r5, pc}
.L4:
	strb	lr, [r3], #1
	tst	r3, #3
	sub	ip, r1, #1
	bne	.L6
.L2:
	cmp	r1, #3
	bls	.L11
	uxtb	lr, r2
	cmp	r1, #15
	orr	lr, lr, lr, asl #8
	orr	lr, lr, lr, asl #16
	bls	.L15
	sub	r4, r1, #16
	add	ip, r3, #16
	mov	r4, r4, lsr #4
	add	r5, r4, #2
	add	r5, r3, r5, lsl #4
.L10:
	str	lr, [ip, #-16]
	add	ip, ip, #16
	str	lr, [ip, #-28]
	str	lr, [ip, #-24]
	str	lr, [ip, #-20]
	cmp	ip, r5
	bne	.L10
	and	r1, r1, #15
	add	ip, r4, #1
	cmp	r1, #3
	add	ip, r3, ip, lsl #4
	bls	.L28
.L9:
	sub	r3, r1, #4
	bic	r3, r3, #3
	add	r3, r3, #4
	add	r3, ip, r3
.L12:
	str	lr, [ip], #4
	cmp	r3, ip
	bne	.L12
	and	r1, r1, #3
.L11:
	cmp	r1, #0
	uxtbne	r2, r2
	addne	r1, r3, r1
	beq	.L29
.L13:
	strb	r2, [r3], #1
	cmp	r3, r1
	bne	.L13
	ldmfd	sp!, {r4, r5, pc}
.L29:
	ldmfd	sp!, {r4, r5, pc}
.L14:
	mov	r3, r0
	b	.L2
.L15:
	mov	ip, r3
	b	.L9
.L28:
	mov	r3, ip
	b	.L11
	.cfi_endproc
	.size __aeabi_memset, . - __aeabi_memset
