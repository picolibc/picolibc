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


#include <asm-leon/asmmacro.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/winmacros.h>
#include <asm-leon/leon.h>
	
#ifndef _SOFT_FLOAT

  	.seg    "text"
	/* ------- */
  	.weak	_fpdis_enable_svt
	.set	_fpdis_enable_svt,__fpdis_enable_svt
  	.weak	_fpdis_enable
	.set	_fpdis_enable,__fpdis_enable
	/* ------- */
	!.global _fpdis_enable,_fpdis_enable_svt
__fpdis_enable_svt:
__fpdis_enable:

	set SPARC_PSR_EF_MASK,%l3
	or %l0,%l3,%l0
	or %l0,0xf00, %l3		! PIL up to 15, enable fpu
	wr %l3,0, %psr		        ! restore the condition flags, enable fpu
	nop
	nop
	nop
	
	mov	%psr, %l3		! check if fpu is present
	set SPARC_PSR_EF_MASK,%l4
	andcc	%l3, %l4, %l3
	bne	4f
	 nop
	
	 ta	0                       ! no fpu present, halt
	
4:
	set     fpustate_current,%l4
	
	ld [%l4],%l4
	set     fpustate_owner,%l5
	ld [%l5],%l5
	cmp	%l4,%l5
	beq	mpfnostore
	 nop
	cmp    %g0,%l5
	 beq	mpfstore
	nop

	FW_STORE(%l5)
	
mpfstore:	
	set     fpustate_owner,%l6
	st %l4,[%l6]
	cmp    %g0,%l4
	 beq	mpfnostore
	nop
	
	FW_LOAD(%l4)

mpfnostore:	
	wr	%l0,0, %psr		! restore the condition flags, enable fpu
	nop
	nop
	nop
	
	jmpl	%l1,  %g0
	rett	%l2
	
  	.seg    "data"

	.global fpustate_default
	.align	8
fpustate_default:
	.long 0; .long 0; .long 0; .long 0; .long 0; .long 0; .long 0; .long 0;
	.long 0; .long 0; .long 0; .long 0; .long 0; .long 0; .long 0; .long 0;
	.long 0; .long 0; .long 0; .long 0; .long 0; .long 0; .long 0; .long 0;
	.long 0; .long 0; .long 0; .long 0; .long 0; .long 0; .long 0; .long 0;
	.long 0; .long 0; .long 0; .long 0; .long 0; .long 0; .long 0; .long 0;
	.global fpustate_owner
fpustate_owner:
	.word	fpustate_default				! pointer to FPU owning context
	
 	.global fpustate_current	
fpustate_current:	
	.word	fpustate_default				! pointer to current threads FPU context
	        
#endif
