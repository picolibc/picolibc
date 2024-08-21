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

; This file contains variants of the same function with different
; instructions.  The generic one, the implementation that comes the
; last after the #else macro, is the most commented.

; Using 128-bit memory operations
#if defined (__ARC64_M128__)

ENTRY (memcpy)
	lsrl.f	r12, r2, 6	; Check size < 64bytes
	beq.d	@.L_write_1_bytes
	movl	r3, r0
.L_write_64_bytes:
	lddl.ab	r4r5, [r1, +16]
	lddl.ab	r6r7, [r1, +16]
	lddl.ab	r8r9, [r1, +16]
	lddl.ab	r10r11, [r1, +16]
	stdl.ab	r4r5, [r3, +16]
	stdl.ab	r6r7, [r3, +16]
	stdl.ab	r8r9, [r3, +16]
	dbnz.d	r12, @.L_write_64_bytes
	stdl.ab	r10r11, [r3, +16]
.L_write_1_bytes:
	;; Handle anything between 15bytes < size < 64bytes
	;; The algorithm has two phases:
	;; - copy 16, 32, or 48 bytes of data using 128bit ops
	;; - copy the remaining 15 bytes of data using a single stdl/lddl pair
	bmsk.f	r2, r2, 5	; Check size == 0
	jeq.d	[blink]
	lsr.f	r12, r2, 4	; Check size < 16bytes
	beq.d	@1f
	xor	r12, r12, 3
	;; R12 can be 3,2, or 1, which are indicating how much data we should
	;; copy: 3 -> 48bytes, 2 -> 32bytes, 1 -> 16bytes.
	;; Zero case shouldn't happen as we check for it above.
	;; Then I use the BI instructions to implement the following code
	;; switch ($R12)
	;;  case 3:
	;;    lddl RA, ...
	;;    stdl RA, ...
	;;  case 2:
	;;    lddl RA, ...
	;;    stdl RA, ...
	;;  case 1:
	;;    lddl RA, ...
	;;    stdl RA, ...
	;;  case 0:
	;;    break
	;; N.B the BI instruction works the other way than I expected, namely
	;; BI's entry 0 is the closest to instruction, hence I need to bit
	;; invert R12 to get the desired behaviour (done by above XOR).
	asl	r12,r12,1
	bi	[r12]
	lddl.ab	r4r5, [r1, +16]
	stdl.ab	r4r5, [r3, +16]
	lddl.ab	r6r7, [r1, +16]
	stdl.ab	r6r7, [r3, +16]
	lddl.ab	r8r9, [r1, +16]
	stdl.ab	r8r9, [r3, +16]
	bmsk.f	r2, r2, 3	; Check size == 0
	jeq.d	[blink]
	subl	r2, r2, 16
	;; We are still having 15 bytes top to transfer, exactly like in the
	;; case of below byte-by-byte transfer.  However, we already transfered
	;; at least 16bytes before, thus, we can create a new 16byte load which
	;; re-reads parts of the already transfer data AND the remaining up to
	;; 15 bytes of data still to be transfered.
	;; The position of the window is controlled by the $r12 which is the
	;; complement of the number of remaining bytes.
	addl	r3, r3, r2
	lddl	r4r5, [r1, r2]
	j_s.d	[blink]
	stdl	r4r5, [r3]
1:
	;; Anything size < 16 we go byte by byte.
	ldb.ab	r4, [r1, +1]
	dbnz.d	r2, @1b
	stb.ab	r4, [r3, +1]
	j_s	[blink]
ENDFUNC (memcpy)

; The 64-bit crunching implementation.
#elif defined (__ARC64_ARCH64__) \
      || (defined (__ARC64_ARCH32__) && defined (__ARC64_LL64__))

; R0: dest
; R1: source
; R2: count
; ret (R0): dest
; clobber: r1, r3, r4r5, r6r7, r8r9, r10r11, r12
ENTRY (memcpy)
	LSRP.f	r12, r2, 5		; counter for 32-byte chunks
	beq.d	@.L_write_31_bytes
	MOVP	r3, r0			; do not clobber the "dest"
.L_write_32_bytes:			; Take care of 32 byte chunks
	LD64.ab	r4, [r1, +8]
	LD64.ab	r6, [r1, +8]
	LD64.ab	r8, [r1, +8]
	LD64.ab	r10,[r1, +8]
	ST64.ab	r4, [r3, +8]
	ST64.ab	r6, [r3, +8]
	ST64.ab	r8, [r3, +8]
	dbnz.d	r12, @.L_write_32_bytes
	ST64.ab	r10, [r3, +8]	; Shove store in delay slot
	bmsk_s	r2, r2, 4		; From now on, we only care for the remainder % 32


; The remainder bits indicating how many more bytes to copy
; .------------------------.
; | b4 | b3 | b2 | b1 | b0 |
; `------------------------'
;   16    8    4    2    1
.L_write_31_bytes:
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
ENDFUNC (memcpy)

#elif defined (__ARC64_ARCH32__)

ENTRY (memcpy)
	lsr.f	r11, r2, 4		; counter for 16-byte chunks
	beq.d	@.L_write_15_bytes
	mov	r3, r0			; work on a copy of "r0"
.L_write_16_bytes:
	ld.ab	r4, [r1, 4]
	ld.ab	r5, [r1, 4]
	ld.ab	r6, [r1, 4]
	ld.ab	r7, [r1, 4]
	st.ab	r4, [r3, 4]
	st.ab	r5, [r3, 4]
	st.ab	r6, [r3, 4]
	dbnz.d	r11, @.L_write_16_bytes
	st.ab	r7, [r3, 4]
	bmsk_s	r2, r2, 3

.L_write_15_bytes:
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
ENDFUNC (memcpy)

#else
# error Unknown configuration
#endif
