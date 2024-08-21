/*
   Copyright (c) 2024, Synopsys, Inc. All rights reserved.

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

#include <sys/asm.h>

; r0 void* dest
; r1 const void* src
; r2 size_t count

; The 64-bit crunching implementation.

#if defined (__ARC64_ARCH32__) && !defined(__ARC64_LL64__)

ENTRY (memmove)

; If the destination is greater than the source
	cmp	r0, r1
	ADDP	r4, r1, r2
; or if the source plus count is smaller than the destination
	cmp.eq r4, r0

; We can safely perform a normal memcpy. Otherwise, we need to perform it
; backwards
	blo.d	@.L_normal_memcpy
	lsr.f	r11, r2, 4		; counter for 16-byte chunks

	ADDP	r3, r0, r2

; Backwards search
; The only thing that changes between memcpy and memmove is copy direction
; in case the dest and src address memory locations overlap
; More detailed information is in the forwards copy and at the end of
; this document

	ADDP	r1, r1, r2
	bmsk_s	r2, r2, 3

	bbit0.d	r2, 1, @1f
	lsr	r5, r2, 2
	ldh.aw	r4, [r1, -2]
	sth.aw	r4, [r3, -2]
1:
	bbit0.d	r2, 0, @1f
	xor	r5, r5, 3
	ldb.aw	r4, [r1, -1]
	stb.aw	r4, [r3, -1]
1:
	asl	r5, r5, 1
	bi	[r5]
	ld.aw	r4,[r1, -4]
	st.aw	r4,[r3, -4]
	ld.aw	r4,[r1, -4]
	st.aw	r4,[r3, -4]
	ld.aw	r4,[r1, -4]
	st.aw	r4,[r3, -4]

; Return if there are no 16 byte chunks
	jeq	[blink]

.L_write_backwards_16_bytes:
	ld.aw	r4, [r1, -4]
	ld.aw	r5, [r1, -4]
	ld.aw	r6, [r1, -4]
	ld.aw	r7, [r1, -4]
	st.aw	r4, [r3, -4]
	st.aw	r5, [r3, -4]
	st.aw	r6, [r3, -4]
	dbnz.d	r11, @.L_write_backwards_16_bytes
	st.aw	r7, [r3, -4]

	j_s	[blink]

.L_normal_memcpy:
	beq.d	@.L_write_forwards_15_bytes
	mov	r3, r0			; work on a copy of "r0"

.L_write_forwards_16_bytes:
	ld.ab	r4, [r1, 4]
	ld.ab	r5, [r1, 4]
	ld.ab	r6, [r1, 4]
	ld.ab	r7, [r1, 4]
	st.ab	r4, [r3, 4]
	st.ab	r5, [r3, 4]
	st.ab	r6, [r3, 4]
	dbnz.d	r11, @.L_write_forwards_16_bytes
	st.ab	r7, [r3, 4]
	bmsk_s	r2, r2, 3

.L_write_forwards_15_bytes:
	bbit0.d	r2, 1, @1f
	lsr	r11, r2, 2
	ldh.ab	r4, [r1, 2]
	sth.ab	r4, [r3, 2]
1:
	bbit0.d	r2, 0, @1f
	xor	r11, r11, 3
	ldb.ab	r4, [r1, 1]
	stb.ab	r4, [r3, 1]
1:
	asl	r11, r11, 1
	bi	[r11]
	ld.ab	r4,[r1, 4]
	st.ab	r4,[r3, 4]
	ld.ab	r4,[r1, 4]
	st.ab	r4,[r3, 4]
	ld	r4,[r1]
	st	r4,[r3]

	j_s	[blink]

ENDFUNC (memmove)

#else

ENTRY (memmove)
; If the destination is greater than the source
	cmp	r0, r1
	ADDP	r4, r1, r2
; or if the source plus count is smaller than the destination
	cmp.eq r4, r0

; We can safely perform a normal memcpy. Otherwise, we need to perform it
; backwards
	blo.d	@.L_normal_memcpy
	LSRP.f	r12, r2, 5		; counter for 32-byte chunks

	ADDP	r3, r0, r2

; Backwards search
; The only thing that changes between memcpy and memmove is copy direction
; in case the dest and src address memory locations overlap
; More detailed information is in the forwards copy and at the end of
; this document

; Set both r0 and r1 to point to the end of each memory location
	ADDP	r1, r1, r2
	bmsk_s	r2, r2, 4

	bbit0.d	r2, 0, @1f
	lsr	r11, r2, 3
	ldb.aw	r4, [r1, -1]
	stb.aw	r4, [r3, -1]
1:
	bbit0.d	r2, 1, @1f
	xor	r11, r11, 3
	ldh.aw	r4, [r1, -2]
	sth.aw	r4, [r3, -2]
1:
	bbit0.d	r2, 2, @1f
	asl	r11, r11, 1
	ld.aw	r4, [r1, -4]
	st.aw	r4, [r3, -4]
1:
	bi	[r11]
	LD64.aw	r4, [r1, -8]
	ST64.aw	r4, [r3, -8]
	LD64.aw	r4, [r1, -8]
	ST64.aw	r4, [r3, -8]
	LD64.aw	r4, [r1, -8]
	ST64.aw	r4, [r3, -8]

; Jump if there are no 32 byte chunks
	jeq	[blink]

.L_write_backwards_32_bytes:			; Take care of 32 byte chunks
#if defined (__ARC64_M128__)

	lddl.aw	r4r5, [r1, -16]
	lddl.aw	r6r7, [r1, -16]

	stdl.aw	r4r5, [r3, -16]
	stdl.aw	r6r7, [r3, -16]
	dbnz	r12, @.L_write_backwards_32_bytes

#elif defined (__ARC64_ARCH64__) || (  defined (__ARC64_ARCH32__) && defined (__ARC64_LL64__) )

	LD64.aw	r4, [r1, -8]
	LD64.aw	r6, [r1, -8]
	LD64.aw	r8, [r1, -8]
	LD64.aw	r10,[r1, -8]

	ST64.aw	r4, [r3, -8]
	ST64.aw	r6, [r3, -8]
	ST64.aw	r8, [r3, -8]
	dbnz.d	r12, @.L_write_backwards_32_bytes
	ST64.aw	r10, [r3, -8]

#else
# error Unknown configuration
#endif

	j_s	[blink]

; Normal memcpy
.L_normal_memcpy:
	;LSRP.f	r12, r2, 5		; Moved up

	beq.d	@.L_write_forwards_31_bytes
	MOVP	r3, r0			; do not clobber the "dest"

.L_write_forwards_32_bytes:			; Take care of 32 byte chunks
#if defined (__ARC64_M128__)

	lddl.ab	r4r5, [r1, +16]
	lddl.ab	r6r7, [r1, +16]

	stdl.ab	r4r5, [r3, +16]
	stdl.ab	r6r7, [r3, +16]
	dbnz	r12, @.L_write_forwards_32_bytes

#elif defined (__ARC64_ARCH64__) || (  defined (__ARC64_ARCH32__) && defined (__ARC64_LL64__) )

	LD64.ab	r4, [r1, +8]
	LD64.ab	r6, [r1, +8]
	LD64.ab	r8, [r1, +8]
	LD64.ab	r10,[r1, +8]
	ST64.ab	r4, [r3, +8]
	ST64.ab	r6, [r3, +8]
	ST64.ab	r8, [r3, +8]
	dbnz.d	r12, @.L_write_forwards_32_bytes
	ST64.ab	r10, [r3, +8]	; Shove store in delay slot

#else
# error Unknown configuration
#endif

	bmsk_s	r2, r2, 4		; From now on, we only care for the remainder % 32


; The remainder bits indicating how many more bytes to copy
; .------------------------.
; | b4 | b3 | b2 | b1 | b0 |
; `------------------------'
;   16    8    4    2    1
.L_write_forwards_31_bytes:
	bbit0.d	r2, 2, @1f		; is b2 set? then copy 4 bytes
	lsr	    r12, r2, 3		; see the notes below
	ld.ab	r4, [r1, 4]
	st.ab	r4, [r3, 4]
1:
	bbit0.d	r2, 1, @1f		; is b1 set? then copy 2 bytes
	xor	    r12, r12, 3
	ldh.ab	r4, [r1, 2]
	sth.ab	r4, [r3, 2]
1:
	bbit0.d	r2, 0, @1f		; is b0 set? then copy 1 byte
	asl	    r12, r12, 1
	ldb.ab	r4, [r1, 1]
	stb.ab	r4, [r3, 1]

; Interpreting bits (b4,b3) [1] and how they correlate to branch index:
;
; (b4,b3) | bytes to copy | branch index
; --------+---------------+-------------
;   00b   |       0       |   3 (11b)
;   01b   |       8       |   2 (10b)
;   10b   |      16       |   1 (01b)
;   11b   |      24       |   0 (00b)
;
; To go from (b4,b3) to branch index, the bits must be flipped.
; In other words, they must be XORed with 11b [2].
;
; Last but not least, "bi" jumps at boundaries of 4. We need to double
; the index to jump 8 bytes [3].
;
; Hence, the 3 operations for calculating the branch index that are spread
; in "bbit0" delay slots:
;
;	lsr	    r12, r2,  3    [1]
;	xor	    r12, r12, 3    [2]
;	asl	    r12, r12, 1    [3]
1:
	bi	    [r12]
	LD64.ab	r4, [r1, 8]
	ST64.ab	r4, [r3, 8]
	LD64.ab	r4, [r1, 8]
	ST64.ab	r4, [r3, 8]
	LD64.ab	r4, [r1, 8]
	ST64.ab	r4, [r3, 8]

	j_s	[blink]

ENDFUNC (memmove)

#endif
