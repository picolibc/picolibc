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

#if defined (__ARC64_ARCH32__)

; 64 bit version has the same working principles, with slightly different
; instructions, so it is more commented

ENTRY (strcmp)
	xor	r12, r12, r12

	mov	r8, NULL_32DT_1

	asl	r9, r8, 7

.L_3_4B_comparison:

	ld.ab	r6, [r0, +4]

	ld.ab	r7, [r1, +4]

#if defined (__ARC64_LL64__)

	ldd.ab	r2r3, [r0, +8]

	ldd.ab	r4r5, [r1, +8]

#else

	ld.ab	r2, [r0, +4]
	ld.ab	r3, [r0, +4]

	ld.ab	r4, [r1, +4]
	ld.ab	r5, [r1, +4]

#endif

	sub	r13, r6, r8
	sub	r10, r2, r8
	sub	r11, r3, r8

	bic	r13, r13, r6
	bic	r10, r10, r2
	bic	r11, r11, r3

	; Look for difference
	sub.f	0, r6, r7
	bset.ne r12, r12, 3

	sub.f	0, r2, r4
	bset.ne r12, r12, 2

	sub.f	0, r3, r5
	bset.ne r12, r12, 1


	; Look for NULL byte
	and.f	r13, r13, r9
	bset.ne	r12, r12, 3

	and.f	r10, r10, r9
	bset.ne	r12, r12, 2

	and.f	r11, r11, r9
	bset.ne	r12, r12, 1

	breq	r12, 0, @.L_3_4B_comparison

; Setup r0, r3 and r5 with the relevant loaded and intermediate values
	mov r0, r11
	mov	r3, r3
	mov	r5, r5

	asr.f	r12, r12, 3

	mov.c	r0, r10
	mov.c	r3, r2
	mov.c	r5, r4

	asr.f	r12, r12, 1

	mov.c	r0, r13
	mov.c	r3, r6
	mov.c	r5, r7


	ffs.f	r10, r0
	xor	r12, r3, r5

	mov.z	r10, 32
	ffs r12, r12

	xbfu 	r10, r10, 0b0111000011
	xbfu 	r12, r12, 0b0111000011


	sub.f	0, r10, r12

	asl.ge	r12, r12, 3

; Difference is first
	lsr.ge	r3, r3, r12
	lsr.ge	r5, r5, r12

	bmsk	r3, r3, 7
	bmsk	r5, r5, 7

	j_s.d	[blink]
	sub	r0, r3, r5


ENDFUNC(strcmp)

#else

ENTRY (strcmp)

	xorl	r12, r12, r12

; Setup byte detector (more information bellow) [1]
	vpack2wl	r8, NULL_32DT_1, NULL_32DT_1
; Set r9 as a copy of r8 for vectorized sub
	asll	r9, r8, 7

.L_3_8B_comparison:

	ldl.ab	r6, [r0, +8]

	ldl.ab	r7, [r1, +8]

; Using 128-bit memory operations
#if defined (__ARC64_M128__)

	lddl.ab	r2r3, [r0, +16]

	lddl.ab	r4r5, [r1, +16]

; The 64-bit crunching implementation.
#elif defined (__ARC64_ARCH64__)

	ldl.ab	r2, [r0, +8]
	ldl.ab	r3, [r0, +8]

	ldl.ab	r4, [r1, +8]
	ldl.ab	r5, [r1, +8]

#else
	# error Unknown configuration
#endif

	subl	r13, r6, r8
	subl	r10, r2, r8
	subl	r11, r3, r8

	bicl	r13, r13, r6
	bicl	r10, r10, r2
	bicl	r11, r11, r3

; Look for difference
	subl.f	0, r6, r7
	bset.ne r12, r12, 3

	subl.f	0, r2, r4
	bset.ne r12, r12, 2

	subl.f	0, r3, r5
	bset.ne r12, r12, 1

; Look for NULL byte
	andl.f	r13, r13, r9
	bset.ne	r12, r12, 3

	andl.f	r10, r10, r9
	bset.ne	r12, r12, 2

	andl.f	r11, r11, r9
	bset.ne	r12, r12, 1

	breq	r12, 0, @.L_3_8B_comparison

; Setup r0, r3 and r5 with the relevant loaded and intermediate values [2]
	; [3]
	movl	r0, r11
	movl	r3, r3
	movl	r5, r5

	asr.f	r12, r12, 3

	movl.c	r0, r10
	movl.c	r3, r2
	movl.c	r5, r4

	asr.f	r12, r12, 1

	movl.c	r0, r13
	movl.c	r3, r6
	movl.c	r5, r7

	ffsl.f	r10, r0		; [5]
	xorl	r12, r3, r5

	movl.z	r10, 64		; [6]
	ffsl	r12, r12	; [8]

	xbful 	r10, r10, 0b0111000011	; [7]
	xbful 	r12, r12, 0b0111000011

; r12 contains position of difference and r10 the position of a NULL byte
; r3 and r5 contain the differing 8 bytes

; Is there a difference?
	subl.f	0, r10, r12
; Multiply the byte position by 8 to get bit shift
	asll.ge	r12, r12, 3

	lsrl.ge	r3, r3, r12
	lsrl.ge	r5, r5, r12

; There is no difference. Up until the NULL byte which must be

	bmskl	r3, r3, 7
	bmskl	r5, r5, 7

	j_s.d	[blink]
	subl	r0, r3, r5


ENDFUNC (strcmp)

#endif

;; One important thing to note, is that we look for the first byte difference on
;; both strings but we only look for the NULL byte in one string.
;; This is because if a NULL byte appears first, it will be the first different
;; byte. If it doesnt, the difference is what matters either way. If there is no
;; difference, the NULL bytes will coincide!
;
;
;; This code uses a common technique for NULL byte detection inside a word.
;; Details on this technique can be found in:
;; (https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord)
;
; In sum, this technique allows for detecting a NULL byte inside any given
; amount of bits by performing the following operation
; 		DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080) [0]
;
; The code above implements this by setting r8 to a 0x01010101... sequence and
; r1 to a 0x80808080... sequence of appropriate length
; As LIMM are 32 bit only, we need to perform MOVHL and ORL [1] operations to
; have the appropriate 64 bit values in place
;
;; Comparison is done 24 bytes at a time, either with 3 64 bit loads or 1 128 bit
;; load and 1 64 bit.
;; If either a NULL byte or a difference between the strings is found, r12 is
;; used to know in which word the NULL/difference is found
;
; With the carry bit from r12, we can use mov.c to only move the appropriate
; registers into the ones we will operate on [2]. We can safely directly move
; the last set of registers without looking at r12, because if they aren't the
; appropriate ones, they will be rewritten afterwards. [3]
;
;; Knowing the registers that contain the relevant information, we only need to
;; look into where the difference and one of the zeros is.
;; This is because, if the zeros are in different places, the difference will
;; either be an earlier difference, or the first zero, so the actual zeros are
;; irrelevant.
;; Zero position is only relevant if there is no difference. And if there is no
;; difference, the zeros have the same position.
;
; So now comes the tricky part. In order to obtain the position of a "first
; NULL byte", we need to understand the NULL byte detection operation.
; It is explained in depth in the link above but in short, it works by first
; setting the highest bit of each byte to 1, if the corresponding byte is either
; 0 or more than 0x80
; Then, it makes the highest bit of each byte 1, if the byte is less than 0x80.
; The last step is to AND these two values (this operation is simplified with
; the SUB, BIC and TST instructions).
;
; This means that the evaluated equation result value has zeros for all non
; zero bytes, except for the NULL bytes. Therefore, we can simply find the
; first non zero bit (counting from bit 0) which will be inside the position of
; the first NULL byte. [5]
;
; One thing to note, is that ffs oddly returns 31/63 if no bit is found, setting
; the zero flag. As there can be that no NULL byte is present on one or both
; strings at this point, we must set r10 and r11 to 32/64 when appropriate. [6]
;
; We can then convert the bit position into the last byte position by looking
; into bits 3 to 5, and shifting 3 bits to the right. This can be combined into
; a single xbful operation. The bottom 000011 represent shift by 3 and the top
; 0111 represents the mask (3 to 5 shifted by 3 is 0 to 2). [7]
;
; To obtain the position of the difference, all we need to do is xor the two
; registers. This way, every equal byte cancels out and all we are left with
; is gibberish in the differing bytes. We can use the same ffs and xbuf
; operations to get the differing byte position.
;
; Note that the order of the operations isnt the same as in this explanation,
; to reduce register dependency between instructions
;
;
; Unlike with r10, we dont need to check the zero flag for r12s' ffs because if
; it is 0, it means there is no difference in the loaded data so any subtraction
; operation will return 0 [8]
;
; There is one optimization that is being overlooked, which is returning 0 if
; there is no difference, but there are NULL bytes anywhere, right after the
; main loop. The reason for this is because the only way this can happen is if
; the strings have the same length AND either are a multiple of 16/8 bytes, or
; the bytes that follow the NULL bytes also match. As this is extremely
; unlikely, it isnt worth it to perform this optimization since it would require
; an extra branch in all runs
;
