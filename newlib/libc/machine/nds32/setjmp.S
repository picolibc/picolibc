/*
Copyright (c) 2013 Andes Technology Corporation.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of the company may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


The setjmp/longjmp for nds32.
The usage of thirty-two 32-bit General Purpose Registers (GPR):
  $r28 : $fp
  $r29 : $gp
  $r30 : $lp
  $r31 : $sp

  caller-save registers: $r0 ~ $r5, $r16 ~ $r23
  callee-save registers: $r6 ~ $r10, $r11 ~ $r14
  reserved for assembler : $r15
  reserved for other use : $r24, $r25, $r26, $r27

Save all callee-save registers and $fp, $gp, $lp and $sp is enough in theory.
For debugging issue, the layout of jum_buf in here should be in sync with GDB.
The $r16 ~ $r19 are used to store D0/D1, keep them for backward-compatible.
*/

/* int setjmp(jmp_buf env);  */
	.text
	.align	2
	.global	setjmp
	.type	setjmp, @function
setjmp:
#ifdef __NDS32_REDUCED_REGS__
	smw.bim	$r6, [$r0], $r10, #0b0000
	addi	$r0, $r0, #32	/* Leave room to keep jum_buf all the same.  */
	smw.bim	$r31, [$r0], $r31, #0b1111
#else
	smw.bim	$r6, [$r0], $r14, #0b0000
	smw.bim	$r16, [$r0], $r19, #0b1111
#endif

	/* Set return value to zero.  */
	movi	$r0, 0
	ret
	.size	setjmp, .-setjmp


/* void longjmp(jmp_buf env, int val);  */
	.text
	.align	2
	.global	longjmp
	.type	longjmp, @function
longjmp:
#ifdef __NDS32_REDUCED_REGS__
	lmw.bim	$r6, [$r0], $r10, #0b0000
	addi    $r0, $r0, #32
	lmw.bim	$r31, [$r0], $r31, #0b1111
#else
	lmw.bim	$r6, [$r0], $r14, #0b0000
	lmw.bim	$r16, [$r0], $r19, #0b1111
#endif
	/* Set val as return value.  If the value val is 0, 1 will be returned
	   instead.  */
	movi	$r0, 1
	cmovn	$r0, $r1, $r1	/* r0=(r1!=0)? r1: r0  */
	ret
	.size	longjmp, .-longjmp
