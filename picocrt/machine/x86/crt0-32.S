/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Mike Haertel and Keith Packard
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

#ifndef CONSTRUCTORS
#define CONSTRUCTORS 1
#endif

	.text
	.section	.text.init.enter
	.globl	_start
	.type	_start, @function
_start:
	.code16
	cs
	lgdtl gdt_desc - _start + 0x10	# use copy of GDT in read-only memory
	mov $1, %eax
	mov %eax, %cr0
	ljmpl $0x10,$_start32
	.code32
_start32:
	#mov	$0x08, %eax		# TSS needed later for handling
	#ltr	%ax			# exceptions and interrupts
	mov	$0x18, %eax		# selector for 32-bit data segment
	mov	%eax, %es		# propagate to all data segments
	mov	%eax, %ss
	mov	%eax, %ds
	mov	%eax, %fs
#ifdef PICOLIBC_TLS
	mov	$0x20, %eax		# selector for thread local storage
#endif
	mov	%eax, %gs
	fninit				# initialize x87 FPU

	/* Check for SSE and enable if present */
	mov	$0x1, %eax
	cpuid
	test	$(1<<25), %edx
	jz	1f
	mov	$0x600, %eax
	mov	%eax, %cr4
1:	

	/* Initialize the stack */
	mov	$__stack, %esp

	/* Initialize data segment */
	pushl	$__data_size
	pushl	$__data_source
	pushl	$__data_start
	call	memcpy
	addl	$12, %esp

	/* Initialize BSS */
	pushl	$__bss_size
	pushl	$0
	pushl	$__bss_start
	call	memset
	addl	$12, %esp

#ifdef PICOLIBC_TLS
	// switch to copy of GDT in writable memory, so we can modify TLS
	// segment base (in case machine lacks MSR_GS_BASE register)
	lgdtl	gdt2_desc

	// call to _set_tls(__tls_base)
	pushl	$__tls_base
	call	_set_tls
	addl	$4, %esp
#endif
	
#if defined(_HAVE_INITFINI_ARRAY) && CONSTRUCTORS
	call	__libc_init_array
#endif

	call	main

#ifdef CRT0_EXIT
	/* keep stack aligned to 16-byte boundary so SSE works */
	push	$0
	push	$0
	push	$0
	push	%eax
	call	exit
#else
1:
	jmp	1b
#endif

	gdt_size = gdt_end - __x86_gdt

	.balign	2
gdt_desc:
	.word gdt_size - 1
	.long __x86_gdt_ro

	// if GDT will not be modified, place it in read-only memory
	// otherwise place it in writable memory, and arrange to first use
	// the read-only copy at early startup
	// linker script computes address of __x86_gdt_ro in that case
#ifndef PICOLIBC_TLS
	.globl 	__x86_gdt_ro
	.globl	__x86_gdt
__x86_gdt_ro:
__x86_gdt:
#else
gdt2_desc:
	.word gdt_size - 1
	.long __x86_gdt

	.data

	.globl 	__x86_gdt
__x86_gdt:
#endif
	.quad 0x0000000000000000	# unused (null selector)
	.quad 0x0000000000000000	# 0x08: space for Task State Segment
	.quad 0x00CF9B000000FFFF        # 0x10: ring 0 32-bit code segment
	.quad 0x00CF93000000FFFF        # 0x18: ring 0 data segment
#ifdef PICOLIBC_TLS
	.quad 0x00CF93000000FFFF	# 0x20: thread local storage
#endif
gdt_end:

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
