/*
   strcpy - copy a string.

   Copyright (c) 2013, 2014 ARM Ltd.
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
/* See strchr-stub.c  */
#else

/* Assumptions:
 *
 * ARMv8-a, AArch64, unaligned accesses, min page size 4k.
 */

/* To test the page crossing code path more thoroughly, compile with
   -DSTRCPY_TEST_PAGE_CROSS - this will force all copies through the slower
   entry path.  This option is not intended for production use.  */

/* Arguments and results.  */
#define dstin		x0
#define srcin		x1

/* Locals and temporaries.  */
#define src		x2
#define dst		x3
#define data1		x4
#define data1w		w4
#define data2		x5
#define data2w		w5
#define has_nul1	x6
#define has_nul2	x7
#define tmp1		x8
#define tmp2		x9
#define tmp3		x10
#define tmp4		x11
#define zeroones	x12
#define data1a		x13
#define data2a		x14
#define pos		x15
#define len		x16
#define to_align	x17

	.macro def_fn f p2align=0
	.text
	.p2align \p2align
	.global \f
	.type \f, %function
\f:
	.endm

	/* NUL detection works on the principle that (X - 1) & (~X) & 0x80
	   (=> (X - 1) & ~(X | 0x7f)) is non-zero iff a byte is zero, and
	   can be done in parallel across the entire word.  */

#define REP8_01 0x0101010101010101
#define REP8_7f 0x7f7f7f7f7f7f7f7f
#define REP8_80 0x8080808080808080

	/* AArch64 systems have a minimum page size of 4k.  We can do a quick
	   page size check for crossing this boundary on entry and if we
	   do not, then we can short-circuit much of the entry code.  We
	   expect early page-crossing strings to be rare (probability of
	   16/MIN_PAGE_SIZE ~= 0.4%), so the branch should be quite
	   predictable, even with random strings.

	   We don't bother checking for larger page sizes, the cost of setting
	   up the correct page size is just not worth the extra gain from
	   a small reduction in the cases taking the slow path.  Note that
	   we only care about whether the first fetch, which may be
	   misaligned, crosses a page boundary - after that we move to aligned
	   fetches for the remainder of the string.  */

#define MIN_PAGE_P2 12
#define MIN_PAGE_SIZE (1 << MIN_PAGE_P2)

def_fn strcpy p2align=6
	/* For moderately short strings, the fastest way to do the copy is to
	   calculate the length of the string in the same way as strlen, then
	   essentially do a memcpy of the result.  This avoids the need for
	   multiple byte copies and further means that by the time we
	   reach the bulk copy loop we know we can always use DWord
	   accesses.  We expect strcpy to rarely be called repeatedly
	   with the same source string, so branch prediction is likely to
	   always be difficult - we mitigate against this by preferring
	   conditional select operations over branches whenever this is
	   feasible.  */
	add	tmp2, srcin, #15
	mov	zeroones, #REP8_01
	and	to_align, srcin, #15
	eor	tmp2, tmp2, srcin
	mov	dst, dstin
	neg	tmp1, to_align
#ifdef STRCPY_TEST_PAGE_CROSS
	b	.Lpage_cross
#else
	/* The first fetch will straddle a (possible) page boundary iff
	   srcin + 15 causes bit[MIN_PAGE_P2] to change value.  A 16-byte
	   aligned string will never fail the page align check, so will
	   always take the fast path.  */
	tbnz	tmp2, #MIN_PAGE_P2, .Lpage_cross
#endif
	ldp	data1, data2, [srcin]
	add	src, srcin, #16
	sub	tmp1, data1, zeroones
	orr	tmp2, data1, #REP8_7f
	sub	tmp3, data2, zeroones
	orr	tmp4, data2, #REP8_7f
	bic	has_nul1, tmp1, tmp2
	bics	has_nul2, tmp3, tmp4
	ccmp	has_nul1, #0, #0, eq	/* NZCV = 0000  */
	b.ne	.Learly_end_found
	stp	data1, data2, [dst], #16
	sub	src, src, to_align
	sub	dst, dst, to_align
	b	.Lentry_no_page_cross

.Lpage_cross:
	bic	src, srcin, #15
	/* Start by loading two words at [srcin & ~15], then forcing the
	   bytes that precede srcin to 0xff.  This means they never look
	   like termination bytes.  */
	ldp	data1, data2, [src], #16
	lsl	tmp1, tmp1, #3	/* Bytes beyond alignment -> bits.  */
	tst	to_align, #7
	csetm	tmp2, ne
#ifdef __AARCH64EB__
	lsl	tmp2, tmp2, tmp1	/* Shift (tmp1 & 63).  */
#else
	lsr	tmp2, tmp2, tmp1	/* Shift (tmp1 & 63).  */
#endif
	orr	data1, data1, tmp2
	orr	data2a, data2, tmp2
	cmp	to_align, #8
	csinv	data1, data1, xzr, lt
	csel	data2, data2, data2a, lt
	sub	tmp1, data1, zeroones
	orr	tmp2, data1, #REP8_7f
	sub	tmp3, data2, zeroones
	orr	tmp4, data2, #REP8_7f
	bic	has_nul1, tmp1, tmp2
	bics	has_nul2, tmp3, tmp4
	ccmp	has_nul1, #0, #0, eq	/* NZCV = 0000  */
	b.ne	.Learly_end_found
	ldp	data1, data2, [src], #16
	sub	tmp1, data1, zeroones
	orr	tmp2, data1, #REP8_7f
	sub	tmp3, data2, zeroones
	orr	tmp4, data2, #REP8_7f
	bic	has_nul1, tmp1, tmp2
	bics	has_nul2, tmp3, tmp4
	ccmp	has_nul1, #0, #0, eq	/* NZCV = 0000  */
	b.ne	.Learly_end_found
	/* We've now checked between 16 and 32 bytes, but not found a null,
	   so we can safely start bulk copying.  Start by refetching the
	   first 16 bytes of the real string; we know this can't trap now.  */
	ldp	data1a, data2a, [srcin]
	stp	data1a, data2a, [dst], #16
	sub	dst, dst, to_align
	/* Everything is now set up, so we can just fall into the bulk
	   copy loop.  */
	/* The inner loop deals with two Dwords at a time.  This has a
	   slightly higher start-up cost, but we should win quite quickly,
	   especially on cores with a high number of issue slots per
	   cycle, as we get much better parallelism out of the operations.  */
.Lmain_loop:
	stp	data1, data2, [dst], #16
.Lentry_no_page_cross:
	ldp	data1, data2, [src], #16
	sub	tmp1, data1, zeroones
	orr	tmp2, data1, #REP8_7f
	sub	tmp3, data2, zeroones
	orr	tmp4, data2, #REP8_7f
	bic	has_nul1, tmp1, tmp2
	bics	has_nul2, tmp3, tmp4
	ccmp	has_nul1, #0, #0, eq	/* NZCV = 0000  */
	b.eq	.Lmain_loop

	/* Since we know we are copying at least 16 bytes, the fastest way
	   to deal with the tail is to determine the location of the
	   trailing NUL, then (re)copy the 16 bytes leading up to that.  */
	cmp	has_nul1, #0
#ifdef __AARCH64EB__
	/* For big-endian, carry propagation (if the final byte in the
	   string is 0x01) means we cannot use has_nul directly.  The
	   easiest way to get the correct byte is to byte-swap the data
	   and calculate the syndrome a second time.  */
	csel	data1, data1, data2, ne
	rev	data1, data1
	sub	tmp1, data1, zeroones
	orr	tmp2, data1, #REP8_7f
	bic	has_nul1, tmp1, tmp2
#else
	csel	has_nul1, has_nul1, has_nul2, ne
#endif
	rev	has_nul1, has_nul1
	clz	pos, has_nul1
	add	tmp1, pos, #72
	add	pos, pos, #8
	csel	pos, pos, tmp1, ne
	add	src, src, pos, lsr #3
	add	dst, dst, pos, lsr #3
	ldp	data1, data2, [src, #-32]
	stp	data1, data2, [dst, #-16]
	ret

	/* The string is short (<32 bytes).  We don't know exactly how
	   short though, yet.  Work out the exact length so that we can
	   quickly select the optimal copy strategy.  */
.Learly_end_found:
	cmp	has_nul1, #0
#ifdef __AARCH64EB__
	/* For big-endian, carry propagation (if the final byte in the
	   string is 0x01) means we cannot use has_nul directly.  The
	   easiest way to get the correct byte is to byte-swap the data
	   and calculate the syndrome a second time.  */
	csel	data1, data1, data2, ne
	rev	data1, data1
	sub	tmp1, data1, zeroones
	orr	tmp2, data1, #REP8_7f
	bic	has_nul1, tmp1, tmp2
#else
	csel	has_nul1, has_nul1, has_nul2, ne
#endif
	rev	has_nul1, has_nul1
	sub	tmp1, src, #7
	sub	src, src, #15
	clz	pos, has_nul1
	csel	src, src, tmp1, ne
	sub	dst, dstin, srcin
	add	src, src, pos, lsr #3		/* Bits to bytes.  */
	add	dst, dst, src
	sub	len, src, srcin
	cmp	len, #8
	b.lt	.Llt8
	cmp	len, #16
	b.lt	.Llt16
	/* 16->32 bytes to copy.  */
	ldp	data1, data2, [srcin]
	ldp	data1a, data2a, [src, #-16]
	stp	data1, data2, [dstin]
	stp	data1a, data2a, [dst, #-16]
	ret
.Llt16:
	/* 8->15 bytes to copy.  */
	ldr	data1, [srcin]
	ldr	data2, [src, #-8]
	str	data1, [dstin]
	str	data2, [dst, #-8]
	ret
.Llt8:
	cmp	len, #4
	b.lt	.Llt4
	/* 4->7 bytes to copy.  */
	ldr	data1w, [srcin]
	ldr	data2w, [src, #-4]
	str	data1w, [dstin]
	str	data2w, [dst, #-4]
	ret
.Llt4:
	cmp	len, #2
	b.lt	.Llt2
	/* 2->3 bytes to copy.  */
	ldrh	data1w, [srcin]
	strh	data1w, [dstin]
	/* Fall-through, one byte (max) to go.  */
.Llt2:
	/* Null-terminated string.  Last character must be zero!  */
	strb	wzr, [dst, #-1]
	ret

	.size	strcpy, . - strcpy
#endif
