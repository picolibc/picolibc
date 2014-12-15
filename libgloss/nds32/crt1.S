/*
Copyright (c) 2013-2014 Andes Technology Corporation.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of the company may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

##==============================================================================
##
##	crt1.S
##
##	nds32 startup code
##
##==============================================================================

#include "syscall_extra.h"

##------------------------------------------------------------------------------
## Vector table setup
##------------------------------------------------------------------------------
	.section	.nds32_init, "ax"
	j	_start

##------------------------------------------------------------------------------
## Startup code implementation
##------------------------------------------------------------------------------
	.section	.text
	.weak	_SDA_BASE_
	.weak	_ITB_BASE_
	.weak	_arg_init
	.weak	__pre_c_init
	.weak	__post_c_init
	.weak	_call_exit
	.global	_start
	.type	_start, @function
	.align	2
_start:
	/* The initialization sequence really does matter !!!
	   The global pointer must be
	   initialized precedence over all others.  */

.L_init_gp:
	/* Initialization for global pointer.  The symbol _SDA_BASE_ is
	   determined by Linker.  SDA stands for Small Data Access.  */
	la	$gp, _SDA_BASE_

#if __NDS32_EXT_EX9__
.L_init_itb:
	/* Initialization for Instruction Table Base (ITB).
	   The symbol _ITB_BASE_ is determined by Linker.
	   Set $ITB only if MSC_CFG.EIT (cr4.b'24) is set.  */
	mfsr	$r0, $MSC_CFG
	srli	$r0, $r0, 24
	andi	$r0, $r0, 0x1
	beqz	$r0, 1f		/* Fall through ?  */
	la	$r0, _ITB_BASE_
	mtusr	$r0, $ITB
1:
#endif

.L_init_sp:
	/* Initialization for stack pointer.  The symbol _stack is defined
	   in linker script.  Make sure $sp is 8-byte aligned.  */
	la	$sp, _stack
#if __NDS32_ISA_V3__
	bitci	$sp, $sp, #7
#else
	movi	$r0, #-8		/* Set $r0 as 0xFFFFFFF8.  */
	and	$sp, $sp, $r0
#endif

#if __NDS32_EXT_FPU_SP__ || __NDS32_EXT_FPU_DP__
.L_init_fpu:
	/* Initialize FPU
	   Set FUCOP_CTL.CP0EN (fucpr.b'0).  */
	mfsr	$r0, $FUCOP_CTL
	ori	$r0, $r0, 0x1
	mtsr	$r0, $FUCOP_CTL
	dsb
	/* According to [bugzilla #9425], set flush-to-zero mode.
	   That is, set $FPCSR.DNZ(b'12) = 1.  */
	FMFCSR	$r0
	ori	$r0, $r0, 0x1000
	FMTCSR	$r0
	dsb
#endif

.L_pre_c_init:
	! call __pre_c_init if provided
	! sample __pre_c_init is in BSP
	la	$r15, __pre_c_init	! load address of __pre_c_init
	beqz	$r15, .L_zero_out_bss	! check existence of __pre_c_init
	jral	$r15			! pre-c-runtime initialization

.L_zero_out_bss:
	/* Zero out the bss section.
	   Equivalence C code for follow part:
	   if (_end == _edata) goto .L_call_main
	   unsinged int *ptr = _edata;
	   while (ptr != _end)
	     *ptr++ = 0
	   $r0 = ptr/_edata
	   $r1 = _end
	   $r2 = 0
	 */
	la	$r0, _edata
	la	$r1, _end
	movi	$r2, #0
	beq	$r0, $r1, .L_cpp_init	/* Branch if no bss.  */
.Lword_clear:
	swi.bi	$r2, [$r0], #4
	bne	$r0, $r1, .Lword_clear

.L_cpp_init:
	/* Call '_init' to invoke constructors.  */
	jal	_init
	/* Register '_fini' into atexit() to invoke destructors when
	   exit() has been reached.  */
	la	$r0, _fini
	jal	atexit

.L_post_c_init:
	! call __post_c_init if provided
	! no sample __post_c_init is provided
	la	$r15, __post_c_init	! load address of __post_c_init
	beqz	$r15, .L_arg_init	! check existence of __post_c_init
	jral	$r15			! post-c-runtime initialization

.L_arg_init:
	! argc/argv initialization if necessary
	la	$r7, _arg_init		! get address of _arg_init
	beqz	$r7, .L_clean_reg	! if there isn't _arg_init, go main
	addi	$sp, $sp, -512		! allocate space for command line
					! and arguments
	move	$r6, $sp		! r6 = buffer addr of cmd line
	move	$r0, $r6		! r0 = buffer addr of cmd line
	syscall	SYS_getcmdline		! get cmd line
	move	$r0, $r6		! r0 = buffer addr of cmd line
	addi	$r1, $r6, 256		! r1 = argv
	jral	$r7			! init argc/argv
	addi	$r1, $r6, 256		! r1 = argv
	b	.L_call_main

.L_clean_reg:
	/* Prepare argc/argv/env for main function.
	   Since there is no operating system so far,
	   we set $r0, $r1, and $r2 to be zero.
	   Note: $r2 already set to zero in .L_zero_out_bss: code fragment.  */
	movi	$r0, 0
	movi	$r1, 0
	movi	$r2, 0

.L_call_main:
	/* Call 'main'.  */
	bal	main

	/* Call _call_exit.  */
	! call _call_exit if necessary; default implementation is in crtexit.c
	la	$r15, _call_exit		! load address of _call_exit
	beqz	$r15, .L_terminate_program	! no _call_exit? go exit
	jral	$r15				! _call_exit will never return

.L_terminate_program:
	/* There are two ways to terminate program:
	    1. User "syscall 0x1" directly.
	    2. Call exit. The  return value $r0 from main() is
	      implicitly passed as argument.

	    Currently, we use option 2 as a solution to follow C99 5.1.2.2.3,
	    but aware that general exit() will do some cleanup procedures
	    which may result in large-memory-footprints.  */

	bal	exit

.L_forever_loop:
	/* Should never return here.  */
	b	.L_forever_loop

	.size	_start, .-_start
