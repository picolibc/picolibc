/*
   Copyright (c) 2015-2024, Synopsys, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1) Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2) Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   3) Neither the name of the Synopsys, Inc., nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

/* This implementation is optimized for performance.  For code size a generic
   implementation of this function from newlib/libc/string/memcpy.c will be
   used.  */
#include <picolibc.h>

#if !defined (__OPTIMIZE_SIZE__) && !defined (__PREFER_SIZE_OVER_SPEED) \
    && !defined (__ARC_RF16__)

#include "asm.h"

#if defined (__ARCHS__)

#ifdef __LITTLE_ENDIAN__
# define SHIFT_1(RX,RY,IMM)	asl	RX, RY, IMM	; <<
# define SHIFT_2(RX,RY,IMM)	lsr	RX, RY, IMM	; >>
# define MERGE_1(RX,RY,IMM)	asl	RX, RY, IMM
# define MERGE_2(RX,RY,IMM)
# define EXTRACT_1(RX,RY,IMM)	and	RX, RY, 0xFFFF
# define EXTRACT_2(RX,RY,IMM)	lsr	RX, RY, IMM
#else
# define SHIFT_1(RX,RY,IMM)	lsr	RX, RY, IMM	; >>
# define SHIFT_2(RX,RY,IMM)	asl	RX, RY, IMM	; <<
# define MERGE_1(RX,RY,IMM)	asl	RX, RY, IMM	; <<
# define MERGE_2(RX,RY,IMM)	asl	RX, RY, IMM	; <<
# define EXTRACT_1(RX,RY,IMM)	lsr	RX, RY, IMM
# define EXTRACT_2(RX,RY,IMM)	lsr	RX, RY, 0x08
#endif

#ifdef __ARC_LL64__
# define PREFETCH_READ(RX)	prefetch	[RX, 56]
# define PREFETCH_WRITE(RX)	prefetchw	[RX, 64]
# define LOADX(DST,RX)		ldd.ab	DST, [RX, 8]
# define STOREX(SRC,RX)		std.ab	SRC, [RX, 8]
# define ZOLSHFT		5
# define ZOLAND			0x1F
#else
# define PREFETCH_READ(RX)	prefetch	[RX, 28]
# define PREFETCH_WRITE(RX)	prefetchw	[RX, 32]
# define LOADX(DST,RX)		ld.ab	DST, [RX, 4]
# define STOREX(SRC,RX)		st.ab	SRC, [RX, 4]
# define ZOLSHFT		4
# define ZOLAND			0xF
#endif


;;; MEMCPY copy memory regions
;;; Input arguments:
;;;   r0 - output memory region
;;;   r1 - input memory region
;;;   r2 - size in bytes
;;; Returns:
;;;   r0 - pointer to the first byte of the output region
;;; Clobber:
;;;   r1, r2, r3, r4, r5, r6, r8r9, r10r11, lp_count

#if !defined (__ARC_UNALIGNED__)

;;; MEMCPY routine for the case when the CPU only accepts ALIGNED
;;; accesses to memory.
ENTRY (memcpy)
	prefetch  [r1]		; Prefetch the read location
	prefetchw [r0]		; Prefetch the write location
	mov.f	0, r2
; if size is zero
	jz.d	[blink]
	mov	r3, r0		; don't clobber ret val

; if size <= 8
	cmp	r2, 8
	bls.d	.Lsmallchunk
	mov.f	lp_count, r2

	and.f	r4, r0, 0x03
	rsub	lp_count, r4, 4
	lpnz	.Laligndestination
	; LOOP BEGIN
	ldb.ab	r5, [r1,1]
	sub	r2, r2, 1
	stb.ab	r5, [r3,1]
.Laligndestination:

; Check the alignment of the source
	and.f	r4, r1, 0x03
	bnz.d	.Lsourceunaligned

; CASE 0: Both source and destination are 32bit aligned
; Convert len to Dwords, unfold x4
	lsr.f	lp_count, r2, ZOLSHFT
	lpnz	.Lcopy32_64bytes
	; LOOP START
	LOADX (r6, r1)
	PREFETCH_READ (r1)
	PREFETCH_WRITE (r3)
	LOADX (r8, r1)
	LOADX (r10, r1)
	LOADX (r4, r1)
	STOREX (r6, r3)
	STOREX (r8, r3)
	STOREX (r10, r3)
	STOREX (r4, r3)
.Lcopy32_64bytes:

	and.f	lp_count, r2, ZOLAND ;Last remaining 31 bytes
.Lsmallchunk:
	lpnz	.Lcopyremainingbytes
	; LOOP START
	ldb.ab	r5, [r1,1]
	stb.ab	r5, [r3,1]
.Lcopyremainingbytes:

	j	[blink]
; END CASE 0

.Lsourceunaligned:
	cmp	r4, 2
	beq.d	.LunalignedOffby2
	sub	r2, r2, 1

	bhi.d	.LunalignedOffby3
	ldb.ab	r5, [r1, 1]

; CASE 1: The source is unaligned, off by 1
	; Hence I need to read 1 byte for a 16bit alignment
	; and 2bytes to reach 32bit alignment
	ldh.ab	r6, [r1, 2]
	sub	r2, r2, 2
	; Convert to words, unfold x2
	lsr.f	lp_count, r2, 3
	MERGE_1 (r6, r6, 8)
	MERGE_2 (r5, r5, 24)
	or	r5, r5, r6

	; Both src and dst are aligned
	lpnz	.Lcopy8bytes_1
	; LOOP START
	ld.ab	r6, [r1, 4]
	prefetch [r1, 28]	;Prefetch the next read location
	ld.ab	r8, [r1,4]
	prefetchw [r3, 32]	;Prefetch the next write location

	SHIFT_1	(r7, r6, 24)
	or	r7, r7, r5
	SHIFT_2	(r5, r6, 8)

	SHIFT_1	(r9, r8, 24)
	or	r9, r9, r5
	SHIFT_2	(r5, r8, 8)

	st.ab	r7, [r3, 4]
	st.ab	r9, [r3, 4]
.Lcopy8bytes_1:

	; Write back the remaining 16bits
	EXTRACT_1 (r6, r5, 16)
	sth.ab	r6, [r3, 2]
	; Write back the remaining 8bits
	EXTRACT_2 (r5, r5, 16)
	stb.ab	r5, [r3, 1]

	and.f	lp_count, r2, 0x07 ;Last 8bytes
	lpnz	.Lcopybytewise_1
	; LOOP START
	ldb.ab	r6, [r1,1]
	stb.ab	r6, [r3,1]
.Lcopybytewise_1:
	j	[blink]

.LunalignedOffby2:
; CASE 2: The source is unaligned, off by 2
	ldh.ab	r5, [r1, 2]
	sub	r2, r2, 1

	; Both src and dst are aligned
	; Convert to words, unfold x2
	lsr.f	lp_count, r2, 3
#ifdef __BIG_ENDIAN__
	asl.nz	r5, r5, 16
#endif
	lpnz	.Lcopy8bytes_2
	; LOOP START
	ld.ab	r6, [r1, 4]
	prefetch [r1, 28]	;Prefetch the next read location
	ld.ab	r8, [r1,4]
	prefetchw [r3, 32]	;Prefetch the next write location

	SHIFT_1	(r7, r6, 16)
	or	r7, r7, r5
	SHIFT_2	(r5, r6, 16)

	SHIFT_1	(r9, r8, 16)
	or	r9, r9, r5
	SHIFT_2	(r5, r8, 16)

	st.ab	r7, [r3, 4]
	st.ab	r9, [r3, 4]
.Lcopy8bytes_2:

#ifdef __BIG_ENDIAN__
	lsr.nz	r5, r5, 16
#endif
	sth.ab	r5, [r3, 2]

	and.f	lp_count, r2, 0x07 ;Last 8bytes
	lpnz	.Lcopybytewise_2
	; LOOP START
	ldb.ab	r6, [r1,1]
	stb.ab	r6, [r3,1]
.Lcopybytewise_2:
	j	[blink]

.LunalignedOffby3:
; CASE 3: The source is unaligned, off by 3
; Hence, I need to read 1byte for achieve the 32bit alignment

	; Both src and dst are aligned
	; Convert to words, unfold x2
	lsr.f	lp_count, r2, 3
#ifdef __BIG_ENDIAN__
	asl.ne	r5, r5, 24
#endif
	lpnz	.Lcopy8bytes_3
	; LOOP START
	ld.ab	r6, [r1, 4]
	prefetch [r1, 28]	;Prefetch the next read location
	ld.ab	r8, [r1,4]
	prefetchw [r3, 32]	;Prefetch the next write location

	SHIFT_1	(r7, r6, 8)
	or	r7, r7, r5
	SHIFT_2	(r5, r6, 24)

	SHIFT_1	(r9, r8, 8)
	or	r9, r9, r5
	SHIFT_2	(r5, r8, 24)

	st.ab	r7, [r3, 4]
	st.ab	r9, [r3, 4]
.Lcopy8bytes_3:

#ifdef __BIG_ENDIAN__
	lsr.nz	r5, r5, 24
#endif
	stb.ab	r5, [r3, 1]

	and.f	lp_count, r2, 0x07 ;Last 8bytes
	lpnz	.Lcopybytewise_3
	; LOOP START
	ldb.ab	r6, [r1,1]
	stb.ab	r6, [r3,1]
.Lcopybytewise_3:
	j	[blink]

ENDFUNC (memcpy)

#else

;;; MEMCPY routine which is used by systems with unaligned memory
;;; accesses.  This is the case for most of ARCHS CPU family.
ENTRY(memcpy)
	prefetch  [r1]		; Prefetch the read location
	prefetchw [r0]		; Prefetch the write location
	mov.f	0, r2
;;; if size is zero
	jz.d	[blink]
	mov	r3, r0		; don't clobber ret val

;;; if size <= 8
	cmp	r2, 8
	bls.d	.Lsmallchunk
	mov.f	lp_count, r2

;;; Convert len to Dwords, unfold x4
	lsr.f	lp_count, r2, ZOLSHFT
	lpnz	.Lcopyfast
	;; LOOP START
	LOADX (r6, r1)
	PREFETCH_READ (r1)
	PREFETCH_WRITE (r3)
	LOADX (r8, r1)
	LOADX (r10, r1)
	LOADX (r4, r1)
	STOREX (r6, r3)
	STOREX (r8, r3)
	STOREX (r10, r3)
	STOREX (r4, r3)
.Lcopyfast:

#ifdef __ARC_LL64__
	and     r2, r2, ZOLAND	;Remaining 31 bytes
	lsr.f   lp_count, r2, 3	;Convert to 64-bit words.
	lpnz	.Lcopy64b
	;; LOOP START
	ldd.ab	r6,[r1,8]
	std.ab	r6,[r3,8]
.Lcopy64b:

	and.f	lp_count, r2, 0x07 ; Last 7 bytes
#else
	and.f	lp_count, r2, ZOLAND
#endif

.Lsmallchunk:
	lpnz	.Lcopyremainingbytes
	;; LOOP START
	ldb.ab	r5, [r1,1]
	stb.ab	r5, [r3,1]
.Lcopyremainingbytes:

	j	[blink]

ENDFUNC(memcpy)
#endif

#endif /* __ARCHS__ */

#endif /* !__OPTIMIZE_SIZE__ && !__PREFER_SIZE_OVER_SPEED */
