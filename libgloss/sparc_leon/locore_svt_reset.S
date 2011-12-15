/*
 * Copyright (c) 2011 Aeroflex Gaisler
 *
 * BSD license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <asm-leon/head.h>
                
	.section .text
	/* ------- */
	.weak	_hardreset_svt
	.set	_hardreset_svt, __hardreset_svt
	/* ------- */
	.global	_hardreset, _hardreset_svt_real
	
! reset entry point for single vector trapping. Try enable svt 
_hardreset_svt_real:
	nop
__hardreset_svt:
	mov	%psr, %l3
	srl	%l3, 24, %g5
	and  	%g5, 3, %g5
	subcc	%g5, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	.L2
	nop
	
	mov	%asr17, %g5		! set svt
	set     1<<13,%g1
	or      %g5,%g1,%g5
	mov     %g5, %asr17
	nop;  nop; nop
	mov	%asr17,%g5 		! check svt
	andcc	%g5, %g1, %g0
	beq	.L2
	 nop

	set _hardreset,%l3
	jmp %l3
         nop
	
.L2:	ta    0x0			! no svt implemented (ether leon2 or svt != 1)
        nop

