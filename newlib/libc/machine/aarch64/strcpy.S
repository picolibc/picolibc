/*
   strcpy/stpcpy - copy a string returning pointer to start/end.

   Copyright (c) 2013, 2014, 2015, 2020-2023 ARM Ltd.
   All Rights Reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
       * Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.
       * Neither the name of the company nor the names of its contributors
         may be used to endorse or promote products derived from this
         software without specific prior written permission.

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
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  */
#if (defined (__OPTIMIZE_SIZE__) || defined (PREFER_SIZE_OVER_SPEED))
/* See strcpy-stub.c  */
#else

/* Assumptions:
 *
 * ARMv8-a, AArch64, Advanced SIMD.
 * MTE compatible.
 */

#include "asmdefs.h"

#define dstin		x0
#define srcin		x1
#define result		x0

#define src		x2
#define dst		x3
#define len		x4
#define synd		x4
#define	tmp		x5
#define shift		x5
#define data1		x6
#define dataw1		w6
#define data2		x7
#define dataw2		w7

#define dataq		q0
#define vdata		v0
#define vhas_nul	v1
#define vend		v2
#define dend		d2
#define dataq2		q1

#ifdef BUILD_STPCPY
# define STRCPY stpcpy
# define IFSTPCPY(X,...) X,__VA_ARGS__
#else
# define STRCPY strcpy
# define IFSTPCPY(X,...)
#endif

/*
   Core algorithm:
   For each 16-byte chunk we calculate a 64-bit nibble mask value with four bits
   per byte. We take 4 bits of every comparison byte with shift right and narrow
   by 4 instruction. Since the bits in the nibble mask reflect the order in
   which things occur in the original string, counting leading zeros identifies
   exactly which byte matched.  */

ENTRY (STRCPY)
	PTR_ARG (0)
	PTR_ARG (1)
	bic	src, srcin, 15
	ld1	{vdata.16b}, [src]
	cmeq	vhas_nul.16b, vdata.16b, 0
	lsl	shift, srcin, 2
	shrn	vend.8b, vhas_nul.8h, 4
	fmov	synd, dend
	lsr	synd, synd, shift
	cbnz	synd, L(tail)

	ldr	dataq, [src, 16]!
	cmeq	vhas_nul.16b, vdata.16b, 0
	shrn	vend.8b, vhas_nul.8h, 4
	fmov	synd, dend
	cbz	synd, L(start_loop)

#ifndef __AARCH64EB__
	rbit	synd, synd
#endif
	sub	tmp, src, srcin
	clz	len, synd
	add	len, tmp, len, lsr 2
	tbz	len, 4, L(less16)
	sub	tmp, len, 15
	ldr	dataq, [srcin]
	ldr	dataq2, [srcin, tmp]
	str	dataq, [dstin]
	str	dataq2, [dstin, tmp]
	IFSTPCPY (add result, dstin, len)
	ret

L(tail):
	rbit	synd, synd
	clz	len, synd
	lsr	len, len, 2
L(less16):
	tbz	len, 3, L(less8)
	sub	tmp, len, 7
	ldr	data1, [srcin]
	ldr	data2, [srcin, tmp]
	str	data1, [dstin]
	str	data2, [dstin, tmp]
	IFSTPCPY (add result, dstin, len)
	ret

	.p2align 4
L(less8):
	subs	tmp, len, 3
	b.lo	L(less4)
	ldr	dataw1, [srcin]
	ldr	dataw2, [srcin, tmp]
	str	dataw1, [dstin]
	str	dataw2, [dstin, tmp]
	IFSTPCPY (add result, dstin, len)
	ret

L(less4):
	cbz	len, L(zerobyte)
	ldrh	dataw1, [srcin]
	strh	dataw1, [dstin]
L(zerobyte):
	strb	wzr, [dstin, len]
	IFSTPCPY (add result, dstin, len)
	ret

	.p2align 4
L(start_loop):
	sub	tmp, srcin, dstin
	ldr	dataq2, [srcin]
	sub	dst, src, tmp
	str	dataq2, [dstin]
L(loop):
	str	dataq, [dst], 32
	ldr	dataq, [src, 16]
	cmeq	vhas_nul.16b, vdata.16b, 0
	umaxp	vend.16b, vhas_nul.16b, vhas_nul.16b
	fmov	synd, dend
	cbnz	synd, L(loopend)
	str	dataq, [dst, -16]
	ldr	dataq, [src, 32]!
	cmeq	vhas_nul.16b, vdata.16b, 0
	umaxp	vend.16b, vhas_nul.16b, vhas_nul.16b
	fmov	synd, dend
	cbz	synd, L(loop)
	add	dst, dst, 16
L(loopend):
	shrn	vend.8b, vhas_nul.8h, 4		/* 128->64 */
	fmov	synd, dend
	sub	dst, dst, 31
#ifndef __AARCH64EB__
	rbit	synd, synd
#endif
	clz	len, synd
	lsr	len, len, 2
	add	dst, dst, len
	ldr	dataq, [dst, tmp]
	str	dataq, [dst]
	IFSTPCPY (add result, dst, 15)
	ret

END (STRCPY)
#endif
