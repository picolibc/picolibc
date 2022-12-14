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

#include "arm-acle-compat.h"

	.thumb
	.syntax unified
	.global __aeabi_memmove
	.type	__aeabi_memmove, %function
	ASM_ALIAS __aeabi_memmove4 __aeabi_memmove
	ASM_ALIAS __aeabi_memmove8 __aeabi_memmove
__aeabi_memmove:
	push	{r4, lr}
	cmp	r0, r1
	bls	3f
	adds	r4, r1, r2
	cmp	r0, r4
	bcs	3f
	subs	r3, r2, #1
	cmp	r2, #0
	beq	2f
	subs	r2, r4, r2
1:
	ldrb	r1, [r2, r3]
	strb	r1, [r0, r3]
	subs	r3, r3, #1
	bcs	1b
2:
#if __ARM_ARCH >= 5
	pop	{r4, pc}
#else
	pop	{r4}
	pop	{r1}
	bx	r1
#endif
3:
	movs	r3, #0
	cmp	r2, #0
	beq	2b
4:
	ldrb	r4, [r1, r3]
	strb	r4, [r0, r3]
	adds	r3, r3, #1
	cmp	r2, r3
	bne	4b
	b	2b
	.size __aeabi_memmove, . - __aeabi_memmove
