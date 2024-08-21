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

; r0 void* ptr
; r1 int ch
; r2 size_t count

#if defined (__ARC64_ARCH32__)

ENTRY (memchr)
	LSRP.f	0, r2, 4 ; counter for 16-byte chunks
	beq.d	@.L_start_1_byte_search

	; Filter for 1 byte
	bmsk	r1, r1, 7
	lsl8	r9, r1

	or	r9, r9, r1
	vpack2hl	r1, r9, r9

	; r1 is now setup with the special 4 byte repetition of the target byte
	; We use r1 because we dont have any more registers free inside the main loop
	; r9 can be repurposed
	mov	r8, NULL_32DT_1
	ror	r9, r8

	xor r3, r3, r3

.L_search_16_bytes:

#if defined (__ARC64_LL64__)

	ldd.ab	r4r5, [r0, +8]
	ldd.ab	r6r7, [r0, +8]

#else

	ld.ab	r4, [r0, +4]
	ld.ab	r5, [r0, +4]
	ld.ab	r6, [r0, +4]
	ld.ab	r7, [r0, +4]

#endif

	xor	r4, r4, r1
	xor	r5, r5, r1
	xor	r6, r6, r1
	xor	r7, r7, r1

	sub	r10, r4, r8
	sub	r11, r5, r8
	sub	r12, r6, r8
	sub	r13, r7, r8

	bic	r10, r10, r4
	bic	r11, r11, r5
	bic	r12, r12, r6
	bic	r13, r13, r7

	tst	r10, r9
	bset.ne	r3, r3, 4

	tst	r11, r9
	bset.ne	r3, r3, 3

	tst	r12, r9
	bset.ne	r3, r3, 2

	tst	r13, r9
	bset.ne	r3, r3, 1

	; Break if found
	brne.d	r3, 0, @.L_found_in_16B

	; Keep going we have more 16 byte chunks
	sub	r2, r2, 16

	brge	r2, 16, @.L_search_16_bytes

	; Reset byte repetition of r1 to 1 single byte
	bmsk	r1, r1, 7

.L_start_1_byte_search:
	; Check if r2 is 0
	breq.d	r2, 0, @.L_byte_not_found
	ldb.ab	r10, [r0, +1]

.L_search_1_byte:

	breq	r10, r1, @.L_found_byte

	dbnz.d	r2, @.L_search_1_byte
	ldb.ab	r10, [r0, +1]

; Byte not found
.L_byte_not_found:
	j.d	[blink]
	MOVP	r0, 0

.L_found_byte:
	j_s.d [blink]
	SUBP	r0, r0, 1

.L_found_in_16B:

	fls	r5, r3 ; [2]

; Select appropriate register to analyze [4]
	mov	r2, r13

; Point r13 to first NULL byte containing double word [3]
	sub2	r0, r0, r5


	asr.f	r3, r3, 3
	mov.c	r2, r12

	asr.f	r3, r3, 1
	mov.c	r2, r11

	asr.f	r3, r3, 1
	mov.c	r2, r10

	and	r2, r2, r9 ; [5]

	ffs	r2, r2 ; [6]

	xbfu 	r2, r2, 0b0111000011 ; [7]

	j.d	[blink]
	add	r0, r0, r2 ; [8]

ENDFUNC (memchr)

#else

ENTRY (memchr)
	lsrl.f	0, r2, 5			; counter for 32-byte chunks
	beq.d	@.L_start_1_byte_search

	; Filter for 1 byte
	bmsk	r1, r1, 7
	lsl8	r9, r1

	or	r9, r9, r1

	vpack2hl	r1, r9, r9
	vpack2wl	r1, r1, r1

	; r1 is now setup with the special 4 byte repetition of the target byte
	; We use r1 because we dont have any more registers free inside the main loop
	; r9 can be repurposed
	vpack2wl	r8, NULL_32DT_1, NULL_32DT_1
	asll	r9, r8, 7

	xorl	r3, r3, r3

.L_search_32_bytes:

; Using 128-bit memory operations
#if defined (__ARC64_M128__)

	lddl.ab	r4r5, [r0, +16]
	lddl.ab	r6r7, [r0, +16]

; The 64-bit crunching implementation.
#elif defined (__ARC64_ARCH64__)

	ldl.ab	r4, [r0, +8]
	ldl.ab	r5, [r0, +8]
	ldl.ab	r6, [r0, +8]
	ldl.ab	r7, [r0, +8]

#else
	# error Unknown configuration
#endif

	xorl	r4, r4, r1
	xorl	r5, r5, r1
	xorl	r6, r6, r1
	xorl	r7, r7, r1

	subl	r10, r4, r8
	subl	r11, r5, r8
	subl	r12, r6, r8
	subl	r13, r7, r8

	bicl	r10, r10, r4
	bicl	r11, r11, r5
	bicl	r12, r12, r6
	bicl	r13, r13, r7

	tstl	r10, r9
	bset.ne	r3, r3, 4

	tstl	r11, r9
	bset.ne	r3, r3, 3

	tstl	r12, r9
	bset.ne	r3, r3, 2

	tstl	r13, r9
	bset.ne	r3, r3, 1

	; Break if found
	brne.d	r3, 0, @.L_found_in_32B

	; Keep going we have more 16 byte chunks
	subl	r2, r2, 32
	brge	r2, 32,	@.L_search_32_bytes

	; Reset byte repetition of r1 to 1 single byte
	bmskl	r1, r1, 7

.L_start_1_byte_search:
	; Check if r2 is 0
	breq.d	r2, 0, @.L_byte_not_found
	ldb.ab	r10, [r0, +1]

.L_search_1_byte:

	breq	r10, r1, @.L_found_byte

	dbnz.d	r2, @.L_search_1_byte
	ldb.ab	r10, [r0, +1]

; Byte not found
.L_byte_not_found:
	j.d	[blink]
	movl	r0, 0

.L_found_byte:
	j_s.d [blink]
	subl	r0, r0, 1

.L_found_in_32B:

	fls	r5, r3 ; [2]

; Select appropriate register to analyze [4]
	movl	r2, r13

; Point r13 to first NULL byte containing double word [3]
	sub3l	r0, r0, r5

	asr.f	r3, r3, 3
	movl.c	r2, r12

	asr.f	r3, r3, 1
	movl.c	r2, r11

	asr.f	r3, r3, 1
	movl.c	r2, r10

	andl	r2, r2, r9 ; [5]

	ffsl	r2, r2 ; [6]

	xbful 	r2, r2, 0b0111000011 ; [7]

	j.d	[blink]
	addl	r0, r0, r2 ; [8]

ENDFUNC (memchr)
#endif

;; This code uses a common technique for NULL byte detection inside a word.
;; Details on this technique can be found in:
;; (https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord)
;
; In sum, this technique allows for detecting a NULL byte inside any given
; amount of bits by performing the following operation
; 		DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080) [0]
;
; The code above implements this by setting r8 to a 0x01010101... sequence and
; r9 to a 0x80808080... sequence of appropriate length
; As LIMM are 32 bit only, we need to perform MOVHL and ORL [1] operations to
; have the appropriate 64 bit values in place
;
; As we want a specific byte and not a NULL byte, we create in r1 a constant
; that is made up of the target byte, on each byte position, that we xor with
; the loaded data to force a NULL byte only if the target byte is present.
; After that we can use the technique directly
;
;; Search is done 32 bytes at a time, either with 64 bit loads or 128 bit loads
;; If the target byte is detected, the position of the double word is encoded
;; in r3, which is eventually used to adjust r0
;
; r3 is set via bset, which means we can simply use a fls to obtain the first
; match (or ffs depending on the values in bset) [2].
; The reason for starting at 1 and not 0 is so r3 encodes how many double
; words to go back, and it wouldnt make sense to go back 0 (the byte would be
; in the next loop iteration).
;
; The first step to take is point r0 to the appropriate double word.
; As the chosen encoded information is how many double words to go back,
; we can simply multiply r3 by 8 and reduce r0 by that amount [3]
;
; Then, we need to place the loaded double word containing the first target byte
; found, into a "common" register we can operate on later [4].
;
; To do this without any jumps, we can shift r3 and perform a conditional mov
; based on the carry flag value.
; The order is very important because the byte can appear in several double
; words, so we want to analyze from last to first.
;
; We can ignore the first asr (which would be asr.f 2, as we started r3 on 1)
; because if r13 isnt the target byte, r2 will always be overwritten so we can
; just decide to start at r7, and overwrite it if needed.
;
; Now comes the tricky part. In order to obtain the first target byte, we need
; to understand the NULL byte detection operation. It is explained in depth in
; the link above but in short, it works by first setting the highest bit of each
; byte to 1, if the corresponding byte is either 0 or more than 0x80
; Then, separately, it makes the highest bit of each byte 1, if the byte is
; less than 0x80. The last step is to AND these two values (this operation is
; simplified with the SUB, BIC and TST instructions).
;
; This means that the evaluated equation result value [5] has zeros for all non
; zero bytes, except for the NULL bytes (which are the target bytes after the
; xor). Therefore, we can simply find the first non zero bit (counting from bit
; 0) which will be inside the position of the first NULL byte.
;
; One thing to note, is that ffs oddly returns 31 if no bit is found, setting
; the zero flag. As r9 is never all 0s at this stage (would mean there is no
; NULL byte and we wouldnt be here) we dont need to worry about that. [6]
;
; We can then convert the bit position into the last byte position by looking
; into bits 3 to 5, and shifting 3 bits to the right. This can be combined into
; a single xbful operation. The bottom 000011 represent shift by 3 and the top
; 0111 represents the mask (3 to 5 shifted by 3 is 0 to 2). We dont need to
; worry about the case where ffs does not find a bit, because we know for sure
; there is at least one NULL byte, and therefore one of the highest bits is set
; to 1 [7]
;
; Finally, we can add the NULL/target byte position inside the loaded double
; word to r0 to obtain the bytes absolute position [8]
;
;
; Some operations are re-ordered such that register dependency is reduced,
; allowing the CPU to run more instructions in parallel
;
