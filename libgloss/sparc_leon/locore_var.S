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

	
	.section .text
	/* ------- */
	.weak	_hardreset_custom_weak
	.set	_hardreset_custom_weak,_hardreset_custom_weak_dummy
	/* ------- */
	.global _nwindows, _leon_version, _nwindows_min1
	
_hardreset_custom_weak_dummy:

!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
! get nwindows and leon version
			
	mov	%psr, %l3
	srl	%l3, 24, %g5
	and  	%g5, 3, %g5
	subcc	%g5, 3, %g0             ! leon3:	3
	bne	1f
	nop
	set     _leon_version,%l0
	set     3,%l1
	st      %l1,[%l0]
	mov	%asr17, %g5		! leon3 has nwindows in %asr17
	ba	2f
1:
	/* other version */
2:
	and	%g5, 0x1f, %g5
	set	_nwindows_min1, %l3
	st	%g5, [%l3]
	add     %g5,1,%g5
	set	_nwindows, %l3
	st	%g5, [%l3]
	set	_nwindows_min2, %l3
	sub     %g5,2,%g5
	st	%g5, [%l3]
	
!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	
	retl
	nop

!''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	.section .data
	.global _nwindows, _leon_version, _nwindows_min1, _nwindows_min2
_nwindows:
	.word 8
_nwindows_min1:	
	.word 7
_nwindows_min2:	
	.word 6
_leon_version:
	.word 3
