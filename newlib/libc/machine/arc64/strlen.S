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

; Code Brief (more info at the bottom):
; Searches the provided string, 32 bytes at a time, using 128 bit loads
; Finds the NULL bytes inside the loaded data
; Analyzes the first NULL byte containing double word and calculates
; size appropriately
;
; R0 const char* ptr (string to measure)
; ret (R0):
;		- unsigned (string size)
;

#if defined (__ARC64_ARCH32__)

ENTRY (strlen)

; Preserve r0 for size calculation when returning
	mov	r13, r0
	xor	r12, r12, r12

; Setup byte detector (more information bellow) [1]
	mov	r8, NULL_32DT_1
; Set r9 as a copy of r8 for vectorized sub
	mov	r9, r8

	asl	r1, r8, 7

.L_4_4B_search:

#if defined (__ARC64_LL64__)

	ldd.ab	r2r3, [r13, +8]
	ldd.ab	r4r5, [r13, +8]

#else

	ld.ab	r2, [r13, +4]
	ld.ab	r3, [r13, +4]
	ld.ab	r4, [r13, +4]
	ld.ab	r5, [r13, +4]

#endif

; NULL byte position is detected and encoded in r12 [0] [9]

	vsub2	r10, r2, r8
	vsub2	r6, r4, r8

	bic	r10, r10, r2
	bic	r11, r11, r3
	bic	r6, r6, r4
	bic	r7, r7, r5

	tst	r10, r1
	bset.ne	r12, r12, 4

	tst	r11, r1
	bset.ne	r12, r12, 3

	tst	r6, r1
	bset.ne	r12, r12, 2

	tst	r7, r1
	bset.ne	r12, r12, 1

	breq.d	r12, 0, @.L_4_4B_search

	fls	r5, r12 ; [2]

; Point r13 to first NULL byte containing double word [3]
	sub2	r13, r13, r5

; Select appropriate register to analyze [4]
	mov	r2, r7

	asr.f	r12, r12, 3
	mov.c	r2, r6

	asr.f	r12, r12, 1
	mov.c	r2, r11

	asr.f	r12, r12, 1
	mov.c	r2, r10

; Point r13 to first NULL byte in selected double word
.L_fix_r13:
	and	r1, r2, r1 ; [5]

	ffs	r1, r1 ; [6]

	xbfu 	r1, r1, 0b0111000011 ; [7]

	add r13, r13, r1 ; [8]

	j_s.d	[blink]
	sub	r0, r13, r0


ENDFUNC (strlen)

#else

ENTRY (strlen)

; Preserve r0 for size calculation when returning
	movl	r13, r0
	xor	r12, r12, r12

; Setup byte detector (more information bellow) [1]
	vpack2wl	r8, NULL_32DT_1, NULL_32DT_1

	asll	r1, r8, 7

.L_4_8B_search:

; Using 128-bit memory operations
#if defined (__ARC64_M128__)

	lddl.ab	r2r3, [r13, +16]
	lddl.ab	r4r5, [r13, +16]

; The 64-bit crunching implementation.
#elif defined (__ARC64_ARCH64__)

	ldl.ab	r2, [r13, +8]
	ldl.ab	r3, [r13, +8]
	ldl.ab	r4, [r13, +8]
	ldl.ab	r5, [r13, +8]

#else
	# error Unknown configuration
#endif

; NULL byte position is detected and encoded in r6 [0] [9]
	subl	r10, r2, r8
	subl	r11, r3, r8
	subl	r6, r4, r8
	subl	r7, r5, r8

	bicl	r10, r10, r2
	bicl	r11, r11, r3
	bicl	r6, r6, r4
	bicl	r7, r7, r5

	tstl	r10, r1
	bset.ne	r12, r12, 4

	tstl	r11, r1
	bset.ne	r12, r12, 3

	tstl	r6, r1
	bset.ne	r12, r12, 2

	tstl	r7, r1
	bset.ne	r12, r12, 1

	breq.d	r12, 0, @.L_4_8B_search

	flsl	r5, r12 ; [2]

; Point r13 to first NULL byte containing double word [3]
	sub3l	r13, r13, r5

; Select appropriate register to analyze [4]
	movl	r2, r7

	asr.f	r12, r12, 3
	movl.c	r2, r6

	asr.f	r12, r12, 1
	movl.c	r2, r11

	asr.f	r12, r12, 1
	movl.c	r2, r10

; Point r13 to first NULL byte in selected double word
.L_fix_r13:
	andl	r1, r2, r1 ; [5]

	ffsl	r1, r1 ; [6]

	xbful 	r1, r1, 0b0111000011 ; [7]

	addl r13, r13, r1 ; [8]

	j_s.d	[blink]
	subl	r0, r13, r0


ENDFUNC (strlen)

#endif

;; This code uses a common technique for NULL byte detection inside a word.
;; Details on this technique can be found in:
;; (https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord)
;
; In sum, this technique allows for detecting a NULL byte inside any given
; amount of bits by performing the following operation
; 		DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080) [0]
;
; The code above implements this by setting r8 to a
; 0x01010101... sequence and r1 to a 0x80808080... sequence of
; appropriate length As LIMM are 32 bit only, we need to perform MOVHL
; and ORL [1] operations to have the appropriate 64 bit values in
; place
;
;; Search is done 32 bytes at a time, either with 64 bit loads or 128
;; bit loads If a NULL byte is detected, the position of the double
;; word is encoded in r12, which is then used to adjust r13
;
; r12 is set via bset, which means we can simply use a fls to obtain
; the first match (or ffs depending on the values in bset) [2].  The
; reason for starting at 1 and not 0 is so r12 encodes how many double
; words to go back, and it wouldnt make sense to go back 0 (the NULL
; would be in the next loop iteration).
;
; The first step to take is point r13 to the appropriate double word.
; As the chosen encoded information is how many double words to go
; back, we can simply multiply r12 by 8 and reduce r13 by that amount
; [3]
;
; Then, we need to place the loaded double word containing the first
; NULL byte into a "common" register we can operate on later [4].
;
; To do this without any jumps, we can shift r12 and perform a
; conditional mov based on the carry flag value.  The order is very
; important because the NULL byte can appear in several double words,
; so we want to analyze from last to first.
;
; We can ignore the first asr (which would be asr.f 2, as we started
; r12 on 1) because if r7 isnt the NULL byte, r2 will always be
; overwritten so we can just decide to start at r7, and overwrite it
; if needed.
;
; Now comes the tricky part. In order to obtain the first NULL byte,
; we need to understand the NULL byte detection operation. It is
; explained in depth in the link above but in short, it works by first
; setting the highest bit of each byte to 1, if the corresponding byte
; is either 0 or more than 0x80 Then, separately, it makes the highest
; bit of each byte 1, if the byte is less than 0x80. The last step is
; to AND these two values (this operation is simplified with the SUB,
; BIC and TST instructions).
;
; This means that the evaluated equation result value [5] has zeros
; for all non zero bytes, except for the NULL bytes. Therefore, we can
; simply find the first non zero bit (counting from bit 0) which will
; be inside the position of the first NULL byte.
;
; One thing to note, is that ffs oddly returns 31 if no bit is found,
; setting the zero flag. As r9 is never all 0s at this stage (would
; mean there is no NULL byte and we wouldnt be here) we dont need to
; worry about that. [6]
;
; We can then convert the bit position into the last byte position by
; looking into bits 3 to 5, and shifting 3 bits to the right. This can
; be combined into a single xbful operation. The bottom 000011
; represent shift by 3 and the top 0111 represents the mask (3 to 5
; shifted by 3 is 0 to 2). We dont need to worry about the case where
; ffs does not find a bit, because we know for sure there is at least
; one NULL byte, and therefore one of the highest bits is set to 1 [7]
;
; Finally, we can add the NULL byte position inside the loaded double
; word to r13 and subtract r0 from r13 to obtain the string size [8]
;
;
; Some operations are re-ordered such that register dependency is
; reduced, allowing the CPU to run more instructions in parallel [9]
;
;
