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
        movl    0(%esp), %edi                   /* argc = *sp */
        leal    4(%esp), %esi                   /* argv = sp + 4 */
        leal    8(%esp,%edi,4), %eax            /* environ = sp + argc * 4 + 8 */
        movl    %eax, environ

#ifndef NO_FLASH
	/* Initialize data segment */
        pushl   $__data_size
        pushl   $__data_source
        pushl   $__data_start
	call	memcpy
        addl    $12, %esp

	/* Initialize BSS */
        pushl   $__bss_size
        pushl   $0
        pushl   $__bss_start
	call	memset
        addl    $12, %esp
#endif

#ifdef __THREAD_LOCAL_STORAGE

#ifdef INIT_TLS
	// call to _init_tls(__tls_base)
	pushl	$__tls_base
	call	_init_tls
        addl    $4, %esp
#endif
	// call to _set_tls(__tls_base)
	pushl	$__tls_base
	call	_set_tls
        addl    $4, %esp
#endif
	
#if defined(__INIT_FINI_ARRAY) && CONSTRUCTORS
	call	__libc_init_array
#endif

        pushl   %esi
        pushl   %edi
	call	main
        addl    $8, %esp

        pushl   %eax
	call	exit

        .size   _start, . - _start

        .text
        .section        .text._set_tls
        .globl  _set_tls
        .type   _set_tls, @function
_set_tls:
        pushl   %ebp
        pushl   %edi
        pushl   %esi
        pushl   %edx
        pushl   %ecx
        pushl   %ebx

        subl    $16, %esp                       # allocate space for user_desc
        movl    %esp, %ebx                      # kernel parameter

        movl    44(%esp), %eax                  # TLS area
        addl    $__x86_tls_tcb_offset, %eax
        movl    %eax, (%eax)                    # self pointer

        movl    $-1, 0(%ebx)                    # entry_number
        movl    %eax, 4(%ebx)                   # base_addr
        movl    $0xfffff, 8(%ebx)               # limit

        # 0b1010001
        #   | |   +--- seg_32bit
        #   | +------- limit_in_pages
        #   +--------- usable

        movl    $0x51, 12(%ebx)                 # flags

        movl    $243, %eax                      # SYS_set_thread_area

        int     $0x80
        
        cmpl    $-4095, %eax
        jae     0f

        movl    (%esp), %edx                    # fetch entry number
        leal    3(,%edx,8), %edx                # gs = 3 + entry_number * 8

        mov     %edx,%gs

0:
        addl    $16, %esp                       # clear local allocation
        popl    %ebx
        popl    %ecx
        popl    %edx
        popl    %esi
        popl    %edi
        popl    %ebp

        ret

/* Reserve space for a TCB. */

        .section        .tls_tail_extra,"aw"
        .type           __tcb_space, @object
        .size           __tcb_space, 4
__tcb_space:
        .zero           4

        .section        .bss.environ
        .globl          environ
        .type           environ, @object
        .size           environ, 4
environ:
        .zero           4

#if defined(__linux__) && defined(__ELF__)
.section .note.GNU-stack,"",%progbits
#endif
