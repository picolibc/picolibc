/*
Copyright (c) 2014, Hesham ALMatary
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

.align  4
.global setjmp
.type   setjmp,@function
setjmp:

	l.sw	4(r3), r1
	l.sw	8(r3), r2
	/* Skip r3-r8 as they are not preserved across function calls */
	l.sw	36(r3), r9
	/* Skip r10 as it's preserved to be used by TLS */ 
	/* Skip r11, setjmp always set it to 0 */
	/* The following set if registers are preserved across function calls */
	l.sw	52(r3), r14
	l.sw	60(r3), r16
	l.sw	68(r3), r18
	l.sw	76(r3), r20
	l.sw	84(r3), r22
	l.sw	92(r3), r24
	l.sw	100(r3), r26
	l.sw	108(r3), r28
	l.sw	116(r3), r30
	/* Save Status Register */
	l.mfspr	r13, r0, 17
	l.sw	124(r3), r13
/* Set result register to 0 and jump */
// Different cases for optional delay slot
#if defined(__OR1K_NODELAY__)
	l.addi	r11, r0, 0
	l.jr	r9
#elif defined(__OR1K_DELAY__)
	l.jr	r9
	l.addi	r11, r0, 0
#else
	l.addi	r11, r0, 0
	l.jr	r9
	l.nop
#endif

.align  4
.global longjmp
.type   longjmp,@function
longjmp:

  /* If the second argument to longjmp is zero, set return address to 1,
  otherwise set it to the value of the second argument */
	l.addi	r11, r0, 1
	l.sfne	r4, r0
	l.bf	1f
	l.nop
	l.addi	r11, r4, 0

  /* Load status register */
1: 
	l.lwz	r15, 124(r3)
	l.mtspr	r0, r15, 17

	l.lwz	r1, 4(r3)
	l.lwz	r2, 8(r3)
	/* Skip r3-r8 as they are not preserved across function calls */
	l.lwz	r9, 36(r3)
	/* Skip r11 as it's always set by longjmp */
	l.lwz	r14, 52(r3)
	l.lwz	r16, 60(r3)
	l.lwz	r18, 68(r3)
	l.lwz	r20, 76(r3)
	l.lwz	r22, 84(r3)
	l.lwz	r24, 92(r3)
	l.lwz	r26, 100(r3)
	l.lwz	r28, 108(r3)

// Different cases for optional delay slot
#if defined(__OR1K_NODELAY__)
	l.lwz	r30, 116(r3)
	l.jr	r9
#elif defined(__OR1K_DELAY__)
	l.jr	r9
	l.lwz	r30, 116(r3)
#else
	l.lwz	r30, 116(r3)
	l.jr	r9
	l.nop
#endif
