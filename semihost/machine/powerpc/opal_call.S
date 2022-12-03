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

	.text
	.section	.text.opal_call
	.align		2
	.p2align	4,,15
/*
 * void
 * opal_call(void *r3, void *r4, void *r5, int call_r6, void *base_r7, void *entry_r8);
 */
#ifdef __PPC64__
	.global		opal_call
	.section	".opd","aw"
	.align		3
opal_call:
	.quad		.opal_call,.TOC.@tocbase,0
	.previous
#else
opal_call:
#endif
	.type		opal_call, @function
.opal_call:
	.cfi_startproc

	/* Save LR */
	mflr		%r0
	std		%r0, 16(%r1)
	/* Save TOC */
	std		%r2, 40(%r1)

	/* Create a new stack frame */
	stdu		%r1,-112(%r1)

	/* Put 'call' in %r0 */
	mr		%r0, %r6

	/* Set OPAL TOC */
	mr		%r2, %r7

	/* Call OPAL */
	mtctr		%r8
	bctrl

	nop

	/* Pop stack frame */
	addi		%r1, %r1, 112

	/* Restore TOC */
	ld		%r2, 40(%r1)

	/* Restore LR */
	ld		%r0, 16(%r1)
	mtlr		%r0

	/* All done */
	blr

	.cfi_endproc
	.size		opal_call,.-.opal_call
