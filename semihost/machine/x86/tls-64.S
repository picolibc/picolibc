/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 TK Chia
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
#include <picolibc.h>

#define MSR_FS_BASE	0xc0000100

	.text
	.globl	_set_tls
	.type	_set_tls, @function
_set_tls:
	/* Assume that MSR_FS_BASE is present and that we can write to it */
	lea	__x86_tls_tcb_offset(%rdi), %rax # compute TCB base
	mov	%rax, (%rax)		# fill in self-pointer
	shld	$32, %rax, %rdx		# write %fs segment base
	mov	$MSR_FS_BASE, %ecx
	wrmsr
	ret
	.size	_set_tls, . - _set_tls

	/*
	 * Reserve space for a TCB
	 * Per the ELF TLS ABI (https://akkadia.org/drepper/tls.pdf), the
	 * TCB must include, at the very least, a pointer to itself
	 */
	.section .tls_tail_extra, "aw", @nobits
	.space	8

#if defined(__linux__) && defined(__ELF__)
	.section .note.GNU-stack, "", @progbits
#endif
