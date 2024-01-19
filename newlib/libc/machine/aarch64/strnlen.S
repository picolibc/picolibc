/* strnlen - calculate the length of a string with limit.

   Copyright (c) 2013, Linaro Limited
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
       * Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.
       * Neither the name of the Linaro nor the
         names of its contributors may be used to endorse or promote products
         derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#if (defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED))
/* See strnlen-stub.c  */
#else

/* Assumptions:
 *
 * ARMv8-a, AArch64, Advanced SIMD.
 * MTE compatible.
 */

#include "asmdefs.h"

#define srcin		x0
#define cntin		x1
#define result		x0

#define src		x2
#define synd		x3
#define	shift		x4
#define tmp		x4
#define cntrem		x5

#define qdata		q0
#define vdata		v0
#define vhas_chr	v1
#define vend		v2
#define dend		d2

/*
   Core algorithm:
   Process the string in 16-byte aligned chunks. Compute a 64-bit mask with
   four bits per byte using the shrn instruction. A count trailing zeros then
   identifies the first zero byte.  */

ENTRY (strnlen)
	PTR_ARG (0)
	SIZE_ARG (1)
	bic	src, srcin, 15
	cbz	cntin, L(nomatch)
	ld1	{vdata.16b}, [src]
	cmeq	vhas_chr.16b, vdata.16b, 0
	lsl	shift, srcin, 2
	shrn	vend.8b, vhas_chr.8h, 4		/* 128->64 */
	fmov	synd, dend
	lsr	synd, synd, shift
	cbz	synd, L(start_loop)
L(finish):
	rbit	synd, synd
	clz	synd, synd
	lsr	result, synd, 2
	cmp	cntin, result
	csel	result, cntin, result, ls
	ret

L(nomatch):
	mov	result, cntin
	ret

L(start_loop):
	sub	tmp, src, srcin
	add	tmp, tmp, 17
	subs	cntrem, cntin, tmp
	b.lo	L(nomatch)

	/* Make sure that it won't overread by a 16-byte chunk */
	tbz	cntrem, 4, L(loop32_2)
	sub	src, src, 16
	.p2align 5
L(loop32):
	ldr	qdata, [src, 32]!
	cmeq	vhas_chr.16b, vdata.16b, 0
	umaxp	vend.16b, vhas_chr.16b, vhas_chr.16b		/* 128->64 */
	fmov	synd, dend
	cbnz	synd, L(end)
L(loop32_2):
	ldr	qdata, [src, 16]
	subs	cntrem, cntrem, 32
	cmeq	vhas_chr.16b, vdata.16b, 0
	b.lo	L(end_2)
	umaxp	vend.16b, vhas_chr.16b, vhas_chr.16b		/* 128->64 */
	fmov	synd, dend
	cbz	synd, L(loop32)
L(end_2):
	add	src, src, 16
L(end):
	shrn	vend.8b, vhas_chr.8h, 4		/* 128->64 */
	sub	result, src, srcin
	fmov	synd, dend
#ifndef __AARCH64EB__
	rbit	synd, synd
#endif
	clz	synd, synd
	add	result, result, synd, lsr 2
	cmp	cntin, result
	csel	result, cntin, result, ls
	ret

END (strnlen)
#endif
