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
	.section	.text._start
	.globl	_start
	.type	_start, @function
_start:

        /* Compute argc, argv and environ.
         *
         * Upon entry, the stack looks like this:
         *
         *   NULL
         *   environ[nenviron-1]
         *   ..
         *   environ[0]
         *   NULL
         *   argv[argc-1]
         *   ...
         *   argv[0]
         *   argc
         */
        movslq  0(%rsp), %r12                   /* argc = *sp */
        leaq    8(%rsp), %r13                   /* argv = sp + 8 */
        leaq    16(%rsp,%r12,8), %r14           /* environ = sp + argc * 8 + 16 */
        movq    %r14, environ

#ifndef NO_FLASH
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
#endif

#ifdef __THREAD_LOCAL_STORAGE

#ifdef INIT_TLS
	// call to _init_tls(__tls_base)
	mov	$__tls_base, %rdi
	call	_init_tls
#endif
	// call to _set_tls(__tls_base)
	mov	$__tls_base, %rdi
	call	_set_tls
#endif
	
#if defined(__INIT_FINI_ARRAY) && CONSTRUCTORS
	call	__libc_init_array
#endif

        movq    %r12, %rdi
        movq    %r13, %rsi
	call	main

	mov	%rax, %rdi
	call	exit

        .size   _start, . - _start

        .text
        .section        .text._set_tls
        .globl  _set_tls
        .type   _set_tls, @function
_set_tls:
        addq    $__x86_tls_tcb_offset, %rdi
        movq    %rdi, (%rdi)                    # fill-in self pointer
        movq    $158, %rax                      # SYS_arch_prtcl
        movq    %rdi, %rsi
        movq    $0x1002, %rdi                   # ARCH_SET_FS
        syscall
        ret

/* Reserve space for a TCB. */

        .section        .tls_tail_extra,"aw"
        .type           __tcb_space, @object
        .size           __tcb_space, 8
__tcb_space:
        .zero           8

        .section        .bss.environ
        .globl          environ
        .type           environ, @object
        .size           environ, 8
environ:
        .zero           8

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
