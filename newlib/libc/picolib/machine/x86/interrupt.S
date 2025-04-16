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

	.section	.text.x86_halt_interrupt,"ax",@progbits
	.globl	x86_halt_interrupt
	.type	x86_halt_interrupt, @function
x86_halt_interrupt:
	jmp	x86_halt_interrupt
	.size	x86_halt_interrupt, .-x86_halt_interrupt

	.weak	x86_machine_check_interrupt
	.set	x86_machine_check_interrupt,x86_halt_interrupt
	.weak	x86_page_fault_interrupt
	.set	x86_page_fault_interrupt,x86_halt_interrupt
	.weak	x86_general_protection_fault_interrupt
	.set	x86_general_protection_fault_interrupt,x86_halt_interrupt
	.weak	x86_stack_segment_fault_interrupt
	.set	x86_stack_segment_fault_interrupt,x86_halt_interrupt
	.weak	x86_segment_not_present_interrupt
	.set	x86_segment_not_present_interrupt,x86_halt_interrupt
	.weak	x86_invalid_opcode_interrupt
	.set	x86_invalid_opcode_interrupt,x86_halt_interrupt
	.weak	x86_breakpoint_interrupt
	.set	x86_breakpoint_interrupt,x86_halt_interrupt
	.weak	x86_nmi_interrupt
	.set	x86_nmi_interrupt,x86_halt_interrupt
	.weak	x86_divide_interrupt
	.set	x86_divide_interrupt,x86_halt_interrupt

	.section	.text.x86_ignore_interrupt,"ax",@progbits
	.globl	x86_ignore_interrupt
	.type	x86_ignore_interrupt, @function
x86_ignore_interrupt:
#ifdef __x86_64
	iretq
#else
	iret
#endif
	.size	x86_ignore_interrupt, .-x86_ignore_interrupt
