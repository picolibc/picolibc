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
	.syntax unified
	.global memset
	.type	memset, %function
memset:
	tst	r0, #3
	stmfd	sp!, {r4, lr}
	beq	10f
	cmp	r2, #0
	sub	r2, r2, #1
	beq	9f
	and	ip, r1, #255
	mov	r3, r0
	b	2f
1:
	cmp	r2, #0
	sub	r2, r2, #1
	beq	9f
2:
	strb	ip, [r3], #1
	tst	r3, #3
	bne	1b
3:
	cmp	r2, #3
	bls	7f
	and	lr, r1, #255
	orr	lr, lr, lr, asl #8
	cmp	r2, #15
	orr	lr, lr, lr, asl #16
	bls	5f
	mov	r4, r2
	add	ip, r3, #16
4:
	sub	r4, r4, #16
	cmp	r4, #15
	str	lr, [ip, #-16]
	str	lr, [ip, #-12]
	str	lr, [ip, #-8]
	str	lr, [ip, #-4]
	add	ip, ip, #16
	bhi	4b
	sub	ip, r2, #16
	bic	ip, ip, #15
	and	r2, r2, #15
	add	ip, ip, #16
	cmp	r2, #3
	add	r3, r3, ip
	bls	7f
5:
	mov	r4, r3
	mov	ip, r2
6:
	sub	ip, ip, #4
	cmp	ip, #3
	str	lr, [r4], #4
	bhi	6b
	sub	ip, r2, #4
	bic	ip, ip, #3
	add	ip, ip, #4
	add	r3, r3, ip
	and	r2, r2, #3
7:
	cmp	r2, #0
	andne	r1, r1, #255
	addne	r2, r3, r2
	beq	9f
8:
	strb	r1, [r3], #1
	cmp	r3, r2
	bne	8b
9:
	ldmfd	sp!, {r4, lr}
	bx	lr
10:
	mov	r3, r0
	b	3b
	.size memset, . - memset
