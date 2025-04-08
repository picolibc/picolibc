/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
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

#ifdef CRT0_SEMIHOST
	
	.macro trap name, str
	.section	.text.\name,"ax",@progbits
	.globl		\name
	.type		\name, @function
\name:
#ifdef __x86_64
	pushq	%rdi
	pushq	%rsi
	movl	$0f, %esi
	subq	$8, %rsp
	leaq	24(%rsp), %rdi
	cld
	call	x86_fault
#else
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%eax
	leal	4(%ebp), %eax
	andl	$-16, %esp
	subl	$24, %esp
	pushl	$0f
	pushl	%eax
	cld
	call	x86_fault
#endif
	.size	\name, . - \name
0:
	.string	"\str"
	.endm
	
	trap x86_divide_interrupt "divide"
	trap x86_nmi_interrupt "nmi"
	trap x86_breakpoint_interrupt "breakpoint"
	trap x86_invalid_opcode_interrupt "invalid opcode"
	trap x86_segment_not_present_interrupt "segment not present"
	trap x86_stack_segment_fault_interrupt "stack segment fault"
	trap x86_general_protection_fault_interrupt "general protection fault"
	trap x86_page_fault_interrupt "page fault"
	trap x86_machine_check_interrupt "machine check"

	.section	.note.GNU-stack,"",@progbits
#endif
