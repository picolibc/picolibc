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
	.type	start, @function
_start:
	.code16
	cs lgdtl	gdt_desc - _start + 0x10
	
	# enable sse and pae
	mov	$0x620, %eax
	mov	%eax, %cr4
	
	# enable long mode by setting EFER
	mov	$0x100, %eax
	xor	%edx, %edx
	mov	$0xc0000080, %ecx
	wrmsr

	# set the page table base address
	mov	$level4pt, %eax
	mov	%eax, %cr3

	# enable paging and protected mode by setting PG and PE
	mov	$0x80000001, %eax
	mov	%eax, %cr0
	
	ljmpl	$0x10,$_start64

	.code64
_start64:
	mov	$0x18, %eax		# selector for 32-bit data segment
	mov	%eax, %es		# propagate to all data segments
	mov	%eax, %ss
	mov	%eax, %ds
	mov	%eax, %fs
	mov	%eax, %gs

	/* Initialize the stack */
	mov	$__stack, %esp

	/* Initialize data segment */
	mov	$__data_size, %rdx
	mov	$__data_source, %rsi
	mov	$__data_start, %rdi
	call	memcpy

	/* Initialize BSS */
	mov	$__bss_size, %rdx
	mov	$0, %rsi
	mov	$__bss_start, %rdi
	call	memset

#ifdef PICOLIBC_TLS
	// call to _set_tls(__tls_base)
	mov	$__tls_base, %rdi
	call	_set_tls
#endif
	
#if defined(_HAVE_INITFINI_ARRAY) && CONSTRUCTORS
	call	__libc_init_array
#endif

	mov	$0, %rdi
	mov	$0, %rsi
	mov	$0, %rdx
	call	main

#ifdef CRT0_EXIT
	mov	%rax, %rdi
	call	exit
#else
1:
	jmp	1b
#endif
	
gdt_desc:
	.word gdt_end - gdt - 1
	.long gdt

gdt:
	.quad 0x0000000000000000	# unused (null selector)
	.quad 0x0000000000000000	# 0x08: space for Task State Segment
	.quad 0x00AF9B000000FFFF        # 0x10: ring 0 64-bit code segment
	.quad 0x00CF93000000FFFF        # 0x18: ring 0 data segment
gdt_end:

	.section .rodata
	/* Page table */
	.balign 4096, 0
level4pt:
	.quad	level3pt + 0x67
	.balign	4096, 0
level3pt:
	.quad	level2pt + 0x67
	.balign	4096, 0
level2pt:
#define page1(base) \
	((0x00000000000000E7) | ((base) << 21))
#define page2(base) \
	page1(base), \
	page1((base)+1)
#define page4(base) \
	page2(base), \
	page2((base)+2)
#define page8(base) \
	page4(base), \
	page4((base)+4)
#define page16(base) \
	page8(base), \
	page8((base)+8)
#define page32(base) \
	page16(base), \
	page16((base)+16)
#define page32(base) \
	page16(base), \
	page16((base)+16)
#define page64(base) \
	page32(base), \
	page32((base)+32)
#define page128(base) \
	page64(base), \
	page64((base)+64)
#define page256(base) \
	page128(base), \
	page128((base)+128)
#define page512(base) \
	page256(base), \
	page256((base)+256)
	.quad page512(0)

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
