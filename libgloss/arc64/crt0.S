/*
   Copyright (c) 2024, Synopsys, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1) Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2) Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   3) Neither the name of the Synopsys, Inc., nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

/*
   The startup code for the ARC family of processors does the following before
   transferring control to user defined main label:
       1.  Set sp to __stack_top (link time variable)
       2.  Zero out the bss section (for uninitialized globals)
   After returning from main, the processor is halted and the pipeline is
   flushed out.
*/

#include "asm.h"
#include "../arc/arc-symbols.h"

	.macro initfp from=0, to=31
	fmvi2s	f0+\from,r0
	.if	\to-\from
	initfp	"(\from+1)",\to
	.endif
	.endm

	.extern main

	.section .text.__startup, "ax", @progbits
	.global	__start
	.type	__start, @function
	.align 4
__start:
	MOVRI_S	sp, STACK_TOP	; Stack address

	; Allow unaligned accesses.
	lr      r2, [0xA]
	bset    r2, r2, 19
	flag    r2

;;;  Clear the bss segment.
	MOVRI_S	r0, SMALL_DATA_BSS_START	; r0 = start of the bss section
	MOVRI_S	r2, SMALL_DATA_BSS_END
	SUBR	r2, r2, r0		; r2 = size of the bss section in bytes
	MOVR_S	r1, 0			; r1 = bytes to fill in
	bl	memset

;;; Clear the registers.
	MOVR_S	r0, 0
	MOVR_S	r1, 0
	MOVR_S	r2, 0
	MOVR_S	r3, 0
	MOVR_S	r4, 0
	MOVR_S	r5, 0
	MOVR_S	r6, 0
	MOVR_S	r7, 0
	MOVR_S	r8, 0
	MOVR_S	r9, 0
	MOVR_S	r10, 0
	MOVR_S	r11, 0
	MOVR_S	r12, 0
	MOVR_S	r13, 0
	MOVR_S	r14, 0
	MOVR_S	r15, 0
	MOVR_S	r16, 0
	MOVR_S	r17, 0
	MOVR_S	r18, 0
	MOVR_S	r19, 0
	MOVR_S	r20, 0
	MOVR_S	r21, 0
	MOVR_S	r22, 0
	MOVR_S	r23, 0
	MOVR_S	r24, 0
	MOVR_S	r25, 0
	MOVR_S	r26, 0
	MOVR_S	r27, 0
	;; SP (r28) is initialized
	MOVR_S	ilink, 0
	MOVR	r30, 0

#if defined(__ARC_FPU_DP__) || defined (__ARC_FPU_SP__)
	initfp	0,31
#endif

	;; Call constructors.
	jl	_init

;;; Setup fini routines to be called from exit
	MOVRI_S	r0, _fini
	jl	atexit

	jl	__setup_argv_and_call_main

	j	exit
	.size	__start, .-__start

;;; arc-main-helper.o object can be used to replace this function
;;; and properly set up arguments and/or other low-level stuff.
	.section .text.__setup_argv_and_call_main,"ax",@progbits
	.weak	__setup_argv_and_call_main
	.type	__setup_argv_and_call_main, @function
	.align 4

__setup_argv_and_call_main:
	PUSHR	blink
;;; Call main() with argv[0] set to "baremetal", argv[1] to NULL and empty envp
	MOVR_S	r0, 1		; Set argc to 1
	ADDPCL	r1,.argv_data	; Set argv to the appropriate pointer
	MOVR_S	r2, 0
	MOVR_S	r3, 0

	MOVRI_S	blink, main
	jl	[blink]

	POPR	blink
	j_s	[blink]

	.align 4
	;; Dummy program name
.prog_name:
	.string		"baremetal"
;; argv data
.argv_data:
	 WORD		@.prog_name
	 WORD		0x00
	.size	__setup_argv_and_call_main, .-__setup_argv_and_call_main

	.section .text._exit_halt,"ax",@progbits
	.global	 _exit_halt
	.type	 _exit_halt, @function
	.align 4
_exit_halt:
	; r0 contains exit code
	mov	r0, r0
	flag	1
	b	_exit_halt
	.size	_exit_halt, .-_exit_halt
