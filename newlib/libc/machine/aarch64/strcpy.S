/*
   strcpy - copy a string.

   Copyright (c) 2013, 2014, ARM Limited
   All rights Reserved.

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
/* See strchr-stub.c  */
#else

/* Assumptions:
 *
 * ARMv8-a, AArch64, unaligned accesses
 */

/* Arguments and results.  */
#define dstin		x0
#define src		x1

/* Locals and temporaries.  */
#define dst		x2
#define data1		x3
#define data1w		w3
#define data2		x4
#define has_nul1	x5
#define has_nul2	x6
#define tmp1		x7
#define tmp2		x8
#define tmp3		x9
#define tmp4		x10
#define zeroones	x11

	.macro def_fn f p2align=0
	.text
	.p2align \p2align
	.global \f
	.type \f, %function
\f:
	.endm

#define REP8_01 0x0101010101010101
#define REP8_7f 0x7f7f7f7f7f7f7f7f
#define REP8_80 0x8080808080808080

	/* Start of critial section -- keep to one 64Byte cache line.  */
def_fn strcpy p2align=6
	mov	zeroones, #REP8_01
	mov	dst, dstin
	ands	tmp1, src, #15
	b.ne	.Lmisaligned
	/* NUL detection works on the principle that (X - 1) & (~X) & 0x80
	   (=> (X - 1) & ~(X | 0x7f)) is non-zero iff a byte is zero, and
	   can be done in parallel across the entire word.  */
	/* The inner loop deals with two Dwords at a time.  This has a
	   slightly higher start-up cost, but we should win quite quickly,
	   especially on cores with a high number of issue slots per
	   cycle, as we get much better parallelism out of the operations.  */
	b	.Lfirst_pass
.Lmain_loop:
	stp	data1, data2, [dst], #16
.Lstartloop_fast:
	ldp	data1, data2, [src], #16
	sub	tmp1, data1, zeroones
	orr	tmp2, data1, #REP8_7f
	sub	tmp3, data2, zeroones
	orr	tmp4, data2, #REP8_7f
	bic	has_nul1, tmp1, tmp2
	bics	has_nul2, tmp3, tmp4
	ccmp	has_nul1, #0, #0, eq	/* NZCV = 0000  */
	b.eq	.Lmain_loop
	/* End of critical section -- keep to one 64Byte cache line.  */

	cbnz	has_nul1, .Lnul_in_data1_fast
.Lnul_in_data2_fast:
	str	data1, [dst], #8
.Lnul_in_data2_fast_after_d1:
	/* For a NUL in data2, we always know that we've moved at least 8
	   bytes, so no need for a slow path.  */
#ifdef __AARCH64EB__
	/* For big-endian only, carry propagation means we can't trust
	   the MSB of the syndrome value calculated above (the byte
	   sequence 01 00 will generate a syndrome of 80 80 rather than
	   00 80).  We get around this by byte-swapping the data and
	   re-calculating.  */
	rev	data2, data2
	sub	tmp1, data2, zeroones
	orr	tmp2, data2, #REP8_7f
	bic	has_nul2, tmp1, tmp2
#endif
	rev	has_nul2, has_nul2
	sub	src, src, #(8+7)
	clz	has_nul2, has_nul2
	lsr	has_nul2, has_nul2, #3		/* Bits to bytes.  */
	sub	dst, dst, #7
	ldr	data2, [src, has_nul2]
	str	data2, [dst, has_nul2]
	ret

.Lnul_in_data1_fast:
	/* Since we know we've already copied at least 8 bytes, we can
	   safely handle the tail with one misaligned dword move.  To do this
	   we calculate the location of the trailing NUL byte and go seven
	   bytes back from that.  */
#ifdef __AARCH64EB__
	/* For big-endian only, carry propagation means we can't trust
	   the MSB of the syndrome value calculated above (the byte
	   sequence 01 00 will generate a syndrome of 80 80 rather than
	   00 80).  We get around this by byte-swapping the data and
	   re-calculating.  */
	rev	data1, data1
	sub	tmp1, data1, zeroones
	orr	tmp2, data1, #REP8_7f
	bic	has_nul1, tmp1, tmp2
#endif
	rev	has_nul1, has_nul1
	sub	src, src, #(16+7)
	clz	has_nul1, has_nul1
	lsr	has_nul1, has_nul1, #3		/* Bits to bytes.  */
	sub	dst, dst, #7
	ldr	data1, [src, has_nul1]
	str	data1, [dst, has_nul1]
	ret

.Lfirst_pass:
	ldp	data1, data2, [src], #16
	sub	tmp1, data1, zeroones
	orr	tmp2, data1, #REP8_7f
	sub	tmp3, data2, zeroones
	orr	tmp4, data2, #REP8_7f
	bic	has_nul1, tmp1, tmp2
	bics	has_nul2, tmp3, tmp4
	ccmp	has_nul1, #0, #0, eq	/* NZCV = 0000  */
	b.eq	.Lmain_loop

	cbz	has_nul1, .Lnul_in_data2_fast
.Lnul_in_data1:
	/* Slow path.  We can't be sure we've moved at least 8 bytes, so
	   fall back to a slow byte-by byte store of the bits already
	   loaded.

	   The worst case when coming through this path is that we've had
	   to copy seven individual bytes to get to alignment and we then
	   have to copy another seven (eight for big-endian) again here.
	   We could try to detect that case (and any case where more than
	   eight bytes have to be copied), but it really doesn't seem
	   worth it.  */
#ifdef __AARCH64EB__
	rev	data1, data1
#else
	/* On little-endian, we can easily check if the NULL byte was
	   in the last byte of the Dword.  For big-endian we'd have to
	   recalculate the syndrome, which is unlikely to be worth it.  */
	lsl	has_nul1, has_nul1, #8
	cbnz	has_nul1, 1f
	str	data1, [dst]
	ret
#endif
1:
	strb	data1w, [dst], #1
	tst	data1, #0xff
	lsr	data1, data1, #8
	b.ne	1b
.Ldone:
	ret

.Lmisaligned:
	cmp	tmp1, #8
	b.ge	2f
	/* There's at least one Dword before we reach alignment, so we can
	   deal with that efficiently.  */
	ldr	data1, [src]
	bic	src, src, #15
	sub	tmp3, data1, zeroones
	orr	tmp4, data1, #REP8_7f
	bics	has_nul1, tmp3, tmp4
	b.ne	.Lnul_in_data1
	str	data1, [dst], #8
	ldr	data2, [src, #8]
	add	src, src, #16
	sub	dst, dst, tmp1
	sub	tmp3, data2, zeroones
	orr	tmp4, data2, #REP8_7f
	bics	has_nul2, tmp3, tmp4
	b.ne	.Lnul_in_data2_fast_after_d1
	str	data2, [dst], #8
	/* We can by-pass the first-pass version of the loop in this case
	   since we know that at least 8 bytes have already been copied.  */
	b	.Lstartloop_fast

2:
	sub	tmp1, tmp1, #16
3:
	ldrb	data1w, [src], #1
	strb	data1w, [dst], #1
	cbz	data1w, .Ldone
	add	tmp1, tmp1, #1
	cbnz	tmp1, 3b
	b	.Lfirst_pass

	.size	strcpy, . - strcpy
#endif
