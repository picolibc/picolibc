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

#if defined (__ARC64_ARCH64__)

; R0: lhs
; R1: rhs
; R2: count
; ret (R0):
;   - lhs < rhs: <0
;   - lhs = rhs:  0
;   - lhs > rhs: >0
ENTRY (memcmp)
	cmpl	  r2, 64
	bls.d	  @.L_compare_1_bytes
	movl	  r3, r0	; "r0" will be used as return value
	; If one is curious why the code below looks like the way it does,
	; there is a documentation at the end of this file.
	lsrl	  r12, r2, 5	; counter for 32-byte chunks
	xor	  r13, r13, r13	; the mask showing inequal registers
	ldl.ab	  r4,  [r3, +8]
	ldl.ab	  r5,  [r1, +8]
.L_compare_32_bytes:
	ldl.ab	  r6,  [r3, +8]
	ldl.ab	  r7,  [r1, +8]
	ldl.ab	  r8,  [r3, +8]
	ldl.ab	  r9,  [r1, +8]
	ldl.ab	  r10, [r3, +8]
	ldl.ab	  r11, [r1, +8]
	xorl.f	  0, r4, r5
	xor.ne	  r13, r13, 0b0001
	xorl.f	  0, r6, r7
	xor.ne	  r13, r13, 0b0010
	xorl.f	  0, r8, r9
	xor.ne	  r13, r13, 0b0100
	xorl.f	  0, r10, r11
	xor.ne	  r13, r13, 0b1000
	brne	  r13, 0, @.L_unequal_find
	ldl.ab	  r4,  [r3, +8]
	dbnz.d	  r12, @.L_compare_32_bytes
	ldl.ab	  r5,  [r1, +8]
	; Adjusting the pointers because of the extra loads in the end
	subl	  r1, r1, 8
	subl	  r3, r3, 8
	bmsk_s	  r2, r2, 4	; any remaining bytes to compare
.L_compare_1_bytes:
	cmp	  r2, 0
	jeq.d	  [blink]
	xor_s     r0, r0, r0
	ldb.ab	  r4, [r3, +1]
	ldb.ab	  r5, [r1, +1]
2:
	sub.f	  r0, r4, r5
	jne.d	  [blink]
	ldb.ab	  r4, [r3, +1]
	dbnz.d	  r2, @2b
	ldb.ab	  r5, [r1, +1]	; this load may read beyond the "count".
	j_s	  [blink]
; At this point, we want to find the _first_ comparison that marked the
; inequality of "lhs" and "rhs".  The rest acts like a multiplexer:
;
; if r4  was not equal to r5  --> r1=r4,  r2=r5
; if r6  was not equal to r7  --> r1=r6,  r2=r7
; if r8  was not equal to r9  --> r1=r8,  r2=r9
; if r10 was not equal to r11 --> r1=r10, r2=r11
; find_different_byte(r1, r2)
;
; About the "bi [n]" (branch index) instruction:  This instruction alters
; next PC (program counter):
;
; next_pc = current_pc + n*4                n*4 is the same as n<<2
;
; In other words, it tells the processor to execute the n'th instruction
; from where we are (assuming all the next instructions are 4 bytes long).
;
; We used this to our benefit.  We made each "case" (unequal_r4r5,
; unequal_r5r6, ...) 16 bytes long (power of 2) and fed "bi" an index
; that is already multiplied by 4 (asl r13, r13, 2).  This translates
; into "bi [n]" jumping to 16-bytes slots.  The last slot we did not
; make 16 bytes long with "nop" because we don't need to address after
; it.
.L_unequal_find:
	ffs	  r13, r13
	asl	  r13, r13, 2
	bi	  [r13]
.L_unequal_r4r5:
	movl	  r1, r4
	b.d	  @.L_diff_byte_in_regs
	movl	  r2, r5
	nop
.L_unequal_r6r7:
	movl	  r1, r6
	b.d	  @.L_diff_byte_in_regs
	movl	  r2, r7
	nop
.L_unequal_r8r9:
	movl	  r1, r8
	b.d	  @.L_diff_byte_in_regs
	movl	  r2, r9
	nop
.L_unequal_r10r11:
	movl	  r1, r10
	movl	  r2, r11
	; fall-through
; If we're here, that means the two operands are not equal.
; 1) First we have to get a mask of their inequality through "xor"
; 2) Then, find the first bit position that they're different: "ffs"
; 3) Depending on the bit position, we want the whole byte containing
;    that bit, in both operands, to become the very first byte (least
;    significant byte), so that we can subtract one from another.
;    Below is an illustration of bit positions and how much we should
;    shift the numbers right:
;    bit position range : (in binary)       | shift right by : (in binary)
;    -------------------+-------------------+----------------+------------
;    [ 0, 7]            : (000000 - 000111) | lsr  0         : 000000
;    [ 8,15]            : (001000 - 001111) | lsr  8         : 001000
;    [16,23]            : (010000 - 010111) | lsr 16         : 010000
;    [24,31]            : (011000 - 011111) | lsr 24         : 011000
;    ...                : ...               | ...            : ...
;    [56,63]            : (111000 - 111111) | lsr 56         : 111000
;    We need to ignore the least 3 bits of "position" to get "shift right"
;    amount: "and 0x38, ..."
; 4) When the bytes are positioned at byte #0, mask out the rest of the
;    bytes and subtract the two operands: lhs - rhs
.L_diff_byte_in_regs:
	xorl	  r0, r1, r2	; (1)
	ffsl	  r0, r0	; (2)
	and	  r0, r0, 0x38	; (3)
	lsrl	  r1, r1, r0	; (3)
	lsrl	  r2, r2, r0	; (3)
	bmsk_s	  r1, r1, 7	; (4)
	bmsk_s	  r2, r2, 7	; (4)
	j_s.d	  [blink]
	subl	  r0, r1, r2	; (4)
ENDFUNC (memcmp)

; __ARC64_ARCH64__
#endif

; The loop at the heart of the "memcmp" function follows some specific
; logic and has gone through a few optimisation filters.  Knowing them
; will help understand the code better.
;
; The comparison logic
; --------------------
; In each loop, we compare 32 bytes of data from "lhs" and "rhs".  Those
; comparisons takes place by using 8 sets of registers:
;
; r4  == r5        xor.f 0, r4,  r5       lhs[i+0]  == rhs[i+0]
; r6  == r7        xor.f 0, r6,  r7       lhs[i+8]  == rhs[i+8]
; r8  == r9        xor.f 0, r8,  r9       lhs[i+16] == rhs[i+16]
; r10 == r11       xor.f 0, r10, r11      lhs[i+24] == rhs[i+32]
;
; The idea is to set a corresponding bit in r13 register for each
; comparison that fails.  The relation between the bits and the
; comparisons are:
;
; r13[0..63] = 0
; r13[0]     = 1 if r4  != r5
; r13[1]     = 1 if r6  != r7
; r13[2]     = 1 if r8  != r9
; r13[3]     = 1 if r10 != r11
;
; If r13 remains 0, the next possible iteration of the loop begins.
; If it is not 0 anymore, the algorithm will be interested in the
; lowest bit that is set to 1.  That is achieved by the "ffs"
; (find first set) instruction.
;
; The loop transformation
; -----------------------
; 1) At first, the loop looks like below:
;
;    .loop
;      ldl.ab  r4,  [r3, +8]
;      ldl.ab  r5,  [r1, +8]
;      ...
;      ldl.ab  r10, [r3, +8]
;      ldl.ab  r11, [r1, +8]
;      xorl.f  0, r4, r5
;      xor.ne  r13, r13, 0b0001
;      ...
;      xorl.f  0, r10, r11
;      xor.ne  r13, r13, 0b1000
;      brne    r13, 0, @.unequal_find
;      dbnz    r12, @.loop
;
; 2) "dbnz" instruction has a delay slot.  To make the code more
;    efficient, we can bring the first 2 instructions of the loop
;    to the end (they will be executed just before the next iteration
;    begins).  To make the logic of the program sound, those 2
;    instructions need to be duplicated before the loop start as well:
;
;      ldl.ab  r4,  [r3, +8]
;      ldl.ab  r5,  [r1, +8]
;    .loop
;      ldl.ab  r6, [r3, +8]
;      ldl.ab  r7, [r1, +8]
;      ...
;      ldl.ab  r10, [r3, +8]
;      ldl.ab  r11, [r1, +8]
;      xorl.f  0, r4, r5
;      xor.ne  r13, r13, 0b0001
;      ...
;      xorl.f  0, r10, r11
;      xor.ne  r13, r13, 0b1000
;      brne    r13, 0, @.unequal_find
;      ldl.ab  r4,  [r3, +8]
;      dbnz.d  r12, @.loop
;      ldl.ab  r5,  [r1, +8]
;
;    There is one more loose end to take care of:  At the last iteration
;    of the loop, there is an extra load into r4 and r5 registers while
;    incrementing the pointers (r3 and r1).  We have to correct for that
;    after the loop:
;
;    .loop:
;      ..
;      brne    r13, 0, @.unequal_find
;      ldl.ab  r4,  [r3, +8]
;      dbnz.d  r12, @.loop
;      ldl.ab  r5,  [r1, +8]
;      subl    r1, r1, 8
;      subl    r3, r3, 8
;
;    One last remark about NOT filling the delay slot of "brne" with
;    "ldl.ab r4, ...".  If the branch is taken, the rest of code that
;    is responsible for finding the differentiating bytes relies that
;    all 8 registers hold the comparison data of the loop.  Putting
;    "ldl.ab r4, ..." into the delay slot of "brne ..." would clobber
;    the "r4" register:
;
;    .loop:
;      ..
;      brne.d  r13, 0, @.unequal_find    --> this branch might be taken
;      ldl.ab  r4,  [r3, +8]             --> clobbers r4
;      dbnz.d  r12, @.loop
;      ldl.ab  r5,  [r1, +8]
;
;    Having "ldl.ab r4, ..." between "brne" and "dbnz" as two control flow
;    altering instructions is good enough.
