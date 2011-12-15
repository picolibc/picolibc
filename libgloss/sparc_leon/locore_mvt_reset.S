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
	.weak	_hardreset_mvt
	.set	_hardreset_mvt,_hardreset_mvt_ram
	/* ------- */
	.global	_hardreset
	
_hardreset_mvt_ram:	
	mov	%psr, %g1
	srl	%g1, 24, %g1
	and  	%g1, 3, %g1
	subcc	%g1, 3, %g0             ! leon2: 0 or 2, leon3:	3
	bne	.L2
	 nop
	
	mov	%asr17, %g1		! set svt
	set     1<<13,%g2
	andn    %g1,%g2,%g1
	mov     %g1, %asr17
	
.L2:	mov	%g0, %g4
	sethi	%hi(_hardreset), %g4
	jmp	%g4+%lo(_hardreset);
         nop

