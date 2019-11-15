/* SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * crt0.S -- PRU startup code
 *
 * Copyright (c) 2018-2019 Dimitar Dimitrov <dimitar@dinux.eu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "newlib.h"

	.extern	main
	.extern	exit

	.text
	.section .init0, "x"
	.global	_start
_start:
	/* Initialize stack pointer. */
	ldi32	sp, _stack_top

	/* DATA and BSS are handled by the loader, so nothing to do here. */

#if !defined(MINRT)
	.extern _do_global_dtors
	/* Ensure destructors get called.  Call is per GNU ABI (i.e. 32-bit
	   function pointers).  But it is also compatible with the TI ABI
	   since GCC supports only little endian PRU.

	   WARNING:  Keep this compatible with both ABIs!  */
	ldi	r14, %pmem(_do_global_dtors)
	call	atexit

	/* Call constructors. Use non-call-clobbered registers. */
	ldi	r5, __init_array_start
	ldi	r6, __init_array_end
ctors_loop:
	qbeq	ctors_done, r5, r6
	/* ABI dictates 16-bit IMEM pointers. */
	lbbo	r7, r5, 0, 2
	call	r7.w0
	add	r5, r5, 2
	jmp	ctors_loop
ctors_done:
#endif

	/* Just in case main() tries to access argc, argv[] and envp. */
	zero	r14, 3 * 4

#if !defined(MINRT)
	.weak __c_args__
	ldi32	r5, __c_args__
	qbeq	__skip_c_args, r5, 0
	lbbo	r14, r5, 0, 4	/* argc */
	add	r15, r5, 4	/* argv */
__skip_c_args:
#endif

	/* Call main */
	call	main

#if !defined(MINRT)
	/* Call exit */
	call	exit
#endif

	/* We should never reach here. */
_crt_exit:
	halt
	jmp	_crt_exit

	/* PRU obviously has no shared libraries, but dso_handle
	   helps to achieve better GCC test coverage. Besides,
	   it should be free with minrt. */
	.section .data
	.global __dso_handle
	.weak __dso_handle
__dso_handle:
	.long	0
