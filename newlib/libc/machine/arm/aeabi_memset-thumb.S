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

#include "acle-compat.h"

	.thumb
	.syntax unified
	.global __aeabi_memset
	.type	__aeabi_memset, %function
	ASM_ALIAS __aeabi_memset4 __aeabi_memset
	ASM_ALIAS __aeabi_memset8 __aeabi_memset
__aeabi_memset:
	push	{r4, r5, r6, lr}
	lsls	r3, r0, #30
	beq	10f
	subs	r4, r1, #1
	cmp	r1, #0
	beq	9f
	lsls	r5, r2, #24
	lsrs	r5, r5, #24
	movs	r3, r0
	movs	r6, #3
	b	2f
1:
	subs	r1, r4, #1
	cmp	r4, #0
	beq	9f
	movs	r4, r1
2:
	adds	r3, r3, #1
	subs	r1, r3, #1
	strb	r5, [r1]
	tst	r3, r6
	bne	1b
3:
	cmp	r4, #3
	bls	7f
	movs	r5, #255
	ands	r5, r2
	lsls	r1, r5, #8
	orrs	r5, r1
	lsls	r1, r5, #16
	orrs	r5, r1
	cmp	r4, #15
	bls	5f
	movs	r6, r4
	subs	r6, r6, #16
	lsrs	r6, r6, #4
	adds	r6, r6, #1
	lsls	r6, r6, #4
	movs	r1, r3
	adds	r3, r3, r6
4:
	str	r5, [r1]
	str	r5, [r1, #4]
	str	r5, [r1, #8]
	str	r5, [r1, #12]
	adds	r1, r1, #16
	cmp	r3, r1
	bne	4b
	movs	r1, #15
	ands	r4, r1
	cmp	r4, #3
	bls	7f
5:
	subs	r6, r4, #4
	lsrs	r6, r6, #2
	adds	r6, r6, #1
	lsls	r6, r6, #2
	movs	r1, r3
	adds	r3, r3, r6
6:
	stmia	r1!, {r5}
	cmp	r3, r1
	bne	6b
	movs	r1, #3
	ands	r4, r1
7:
	cmp	r4, #0
	beq	9f
	lsls	r2, r2, #24
	lsrs	r2, r2, #24
	adds	r4, r3, r4
8:
	strb	r2, [r3]
	adds	r3, r3, #1
	cmp	r4, r3
	bne	8b
9:
#if __ARM_ARCH >= 5
	pop	{r4, r5, r6, pc}
#else
	pop	{r4, r5, r6}
	pop	{r1}
	bx	r1
#endif
10:
	movs	r3, r0
	movs	r4, r1
	b	3b
	.size __aeabi_memset, . - __aeabi_memset
