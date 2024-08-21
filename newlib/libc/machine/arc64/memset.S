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

ENTRY (memset)
	;; Assemble 128b token
	bmsk_s	r1, r1, 7
	lsl8	r3, r1
	or_s	r1, r1, r3
	lsl16	r3, r1
	or	r6, r1, r3
	addhl	r6, r6, r6
	movl	r7, r6

	lsrl.f	r5, r2, 6
	beq.d	@.L_write_63_bytes
	movl	r4, r0
.L_write_64_bytes:
	stdl.ab	r6r7, [r4, +16]
	stdl.ab	r6r7, [r4, +16]
	stdl.ab	r6r7, [r4, +16]
	dbnz.d	r5, @.L_write_64_bytes
	stdl.ab	r6r7, [r4, +16]
	bmsk_s	r2, r2, 5

.L_write_63_bytes:
	bbit0.d	r2, 3, @1f
	lsr	r3, r2, 4
	stl.ab	r6, [r4, 8]
1:
	bbit0.d	r2, 2, @1f
	xor	r3, r3, 3
	st.ab	r6, [r4, 4]
1:
	bbit0	r2, 1, @1f
	sth.ab	r6, [r4, 2]
1:
	bbit0	r2, 0, @1f
	stb.ab	r6, [r4, 1]
1:
	bi	[r3]
	stdl.ab	r6r7,[r4, 16]
	stdl.ab	r6r7,[r4, 16]
	stdl.ab	r6r7,[r4, 16]

	j_s	[blink]

.L_write_1_bytes:
	breq	r2, 0, @.L_return
	dbnz.d	r2, @.
	stb.ab	r1, [r4, +1]
.L_return:
	j_s	[blink]
ENDFUNC (memset)

; The generic 64-bit implementation without any frills.
#elif defined (__ARC64_ARCH64__) || defined (__ARC64_LL64__)

#if defined (__ARC64_ARCH32__)
# define MOVH  mov	r7,r6
#elif defined (__ARC64_ARCH64__)
# define MOVH  addhl	r6,r6,r6
#else
# error Please use either 32-bit or 64-bit version of arc64 compiler
#endif

; R0: dest
; R1: ch
; R2: count
; ret (R0): dest
ENTRY (memset)
	;; Assemble the bytes to 64bit words
	bmsk_s	r1, r1, 7		; treat it like unsigned char
	lsl8	r3, r1
	or_s	r1, r1, r3
	lsl16	r3, r1
	or	r6, r1, r3
	MOVH

	LSRP.f	r5, r2, 5		; counter for 32-byte chunks
	beq.d	@.L_write_31_bytes
	MOVP	r4, r0			; work on a copy of "r0"
.L_write_32_bytes:
	ST64.ab	r6, [r4, +8]
	ST64.ab	r6, [r4, +8]
	ST64.ab	r6, [r4, +8]
	dbnz.d	r5, @.L_write_32_bytes
	ST64.ab	r6, [r4, +8]
	bmsk_s	r2, r2, 4

.L_write_31_bytes:
	bbit0.d	r2, 2, @1f
	lsr	r3, r2, 3
	st.ab	r6, [r4, 4]
1:
	bbit0.d	r2, 1, @1f
	xor	r3, r3, 3
	sth.ab	r6, [r4, 2]
1:
	bbit0	r2, 0, @1f
	stb.ab	r6, [r4, 1]
1:
	bi	[r3]
	ST64.ab	r6,[r4, 8]
	ST64.ab	r6,[r4, 8]
	ST64.ab	r6,[r4, 8]

	j_s	[blink]
ENDFUNC (memset)

#elif defined (__ARC64_ARCH32__)
ENTRY (memset)
	;; Assemble the bytes to 32bit words
	bmsk_s	r1, r1, 7		; treat it like unsigned char
	lsl8	r3, r1
	or_s	r1, r1, r3
	lsl16	r3, r1
	or	r6, r1, r3

	lsr.f	r5, r2, 4		; counter for 16-byte chunks
	beq.d	@.L_write_15_bytes
	mov	r4, r0			; work on a copy of "r0"
.L_write_16_bytes:
	st.ab	r6, [r4, 4]
	st.ab	r6, [r4, 4]
	st.ab	r6, [r4, 4]
	dbnz.d	r5, @.L_write_16_bytes
	st.ab	r6, [r4, 4]
	bmsk_s	r2, r2, 3

.L_write_15_bytes:
	bbit0.d	r2, 1, @1f
	lsr	r3, r2, 2
	sth.ab	r6, [r4, 2]
1:
	bbit0.d	r2, 0, @1f
	xor	r3, r3, 3
	stb.ab	r6, [r4, 1]
1:
	bi	[r3]
	st.ab	r6,[r4, 4]
	st.ab	r6,[r4, 4]
	st.ab	r6,[r4, 4]

	j_s	[blink]
ENDFUNC (memset)
#else
# error Unknown configuration
#endif
