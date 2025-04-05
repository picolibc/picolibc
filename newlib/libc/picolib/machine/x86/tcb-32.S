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

	.text
	.weak	__set_tcb
	.type	__set_tcb, @function
__set_tcb:
	/*
	 * Assume that the %gs selector value refers to a slot in the GDT
	 * dedicated to thread local storage; modify the GDT slot's base
	 *
	 * NOTE: this procedure only works if our program is running in
	 * kernel mode (ring 0) and can directly write to the GDT
	 *
	 * If we are running unprivileged, then a syscall is likely needed
	 * (e.g. set_thread_area on Linux/x86)
	 */
	push	%edx
	push	%edx
	sgdt	2(%esp)			# read GDTR to get pointer to GDT
	pop	%edx
	pop	%edx
	xor	%ecx, %ecx
	mov	%gs, %cx		# get index into GDT
	shr	$3, %ecx
	jbe	9f
	lea	(%edx, %ecx, 8), %ecx	# compute pointer to GDT entry
	mov	4(%esp), %eax		# get TCB base
	mov	%eax, (%eax)		# fill in self-pointer
	shld	$8, %eax, %edx		# split TCB address into two parts
	shl	$8, %eax
	mov	$0xff, %al		# update GDT entry
	mov	%eax, 1(%ecx)
	mov	%dl, 7(%ecx)
	mov	%gs, %ecx		# reload %gs descriptor cache
	mov	%ecx, %gs
	ret
9:
	ud2
	.size	__set_tcb, . - __set_tcb

#if defined(__linux__) && defined(__ELF__)
	.section .note.GNU-stack, "", @progbits
#endif
