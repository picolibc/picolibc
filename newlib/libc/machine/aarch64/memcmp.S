/*
 * Copyright (c) 2017 ARM Ltd
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

#if (defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED))
/* See memcmp-stub.c  */
#else

/* Assumptions:
 *
 * ARMv8-a, AArch64, unaligned accesses.
 */

/* Parameters and result.  */
#define src1		x0
#define src2		x1
#define limit		x2
#define result		w0

/* Internal variables.  */
#define data1		x3
#define data1w		w3
#define data2		x4
#define data2w		w4
#define tmp1		x5

        .macro def_fn f p2align=0
        .text
        .p2align \p2align
        .global \f
        .type \f, %function
\f:
        .endm

/* Small inputs of less than 8 bytes are handled separately.  This allows the
   main code to be sped up using unaligned loads since there are now at least
   8 bytes to be compared.  If the first 8 bytes are equal, align src1.
   This ensures each iteration does at most one unaligned access even if both
   src1 and src2 are unaligned, and mutually aligned inputs behave as if
   aligned.  After the main loop, process the last 8 bytes using unaligned
   accesses.  */

def_fn memcmp p2align=6
	subs	limit, limit, 8
	b.lo	.Lless8

	/* Limit >= 8, so check first 8 bytes using unaligned loads.  */
	ldr	data1, [src1], 8
	ldr	data2, [src2], 8
	and	tmp1, src1, 7
	add	limit, limit, tmp1
	cmp	data1, data2
	bne	.Lreturn

	/* Align src1 and adjust src2 with bytes not yet done.  */
	sub	src1, src1, tmp1
	sub	src2, src2, tmp1

	subs	limit, limit, 8
	b.ls	.Llast_bytes

	/* Loop performing 8 bytes per iteration using aligned src1.
	   Limit is pre-decremented by 8 and must be larger than zero.
	   Exit if <= 8 bytes left to do or if the data is not equal.  */
	.p2align 4
.Lloop8:
	ldr	data1, [src1], 8
	ldr	data2, [src2], 8
	subs	limit, limit, 8
	ccmp	data1, data2, 0, hi  /* NZCV = 0b0000.  */
	b.eq	.Lloop8

	cmp	data1, data2
	bne	.Lreturn

	/* Compare last 1-8 bytes using unaligned access.  */
.Llast_bytes:
	ldr	data1, [src1, limit]
	ldr	data2, [src2, limit]

	/* Compare data bytes and set return value to 0, -1 or 1.  */
.Lreturn:
#ifndef __AARCH64EB__
	rev	data1, data1
	rev	data2, data2
#endif
	cmp     data1, data2
.Lret_eq:
	cset	result, ne
	cneg	result, result, lo
        ret

	.p2align 4
	/* Compare up to 8 bytes.  Limit is [-8..-1].  */
.Lless8:
	adds	limit, limit, 4
	b.lo	.Lless4
	ldr	data1w, [src1], 4
	ldr	data2w, [src2], 4
	cmp	data1w, data2w
	b.ne	.Lreturn
	sub	limit, limit, 4
.Lless4:
	adds	limit, limit, 4
	beq	.Lret_eq
.Lbyte_loop:
	ldrb	data1w, [src1], 1
	ldrb	data2w, [src2], 1
	subs	limit, limit, 1
	ccmp	data1w, data2w, 0, ne	/* NZCV = 0b0000.  */
	b.eq	.Lbyte_loop
	sub	result, data1w, data2w
	ret

	.size	memcmp, . - memcmp
#endif
