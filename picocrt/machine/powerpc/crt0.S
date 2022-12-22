/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define PPC_BIT(bit)            (0x8000000000000000ULL >> (bit))
	.text
	.section	.text.init.enter
	.global _start
	.type	_start, @function
_start:

	/* Where are we? */
	bl	here
here:	mflr	%r2

	/* Get stack pointer */
	addi	%r1,%r2,stackaddr - here
	ld	%r1,0(%r1)

#ifdef __PPC64__
	/* Figure out the initial TOC value */
	addi	%r2,%r2,toc-here
	ld	%r2,0(%r2)
#endif
	
	mfmsr	%r0
	/* Enable FPU */
	ori	%r0,%r0,PPC_BIT(50)
	/* Enable VSX */
	oris	%r0,%r0,PPC_BIT(40)>>16
	/* Enable Altivec */
	oris	%r0,%r0,PPC_BIT(38)>>16
	mtmsr	%r0

	/*
	 *   r8 is the OPAL base
	 *   r9 is the OPAL entry point point
	 */
	mr	%r3, %r8
	mr	%r4, %r9
	bl	_cstart
	.align	3
#ifdef __PPC64__
toc:	
	.quad 	.TOC.@tocbase

stackaddr:
	.quad	__stack

#else
stackaddr:
	.long	__stack
#endif
	
