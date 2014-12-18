/*
Copyright (c) 2013 Andes Technology Corporation.
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
##	crt0.S
##
##	nds32 startup code
##
##==============================================================================


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
	.weak	_FP_BASE_
	.global	_start
	.type	_start, @function
	.align	2
_start:
.L_fp_gp_lp_init:
	/* Initialization for $fp, $gp, and $lp. The _SDA_BASE_ location
	   stands for Small Data Access.  */
	la	$fp, _FP_BASE_
	la	$gp, _SDA_BASE_
	movi	$lp, #0

.L_stack_init:
	/* Initialization for $sp and make sure it is 8-byte aligned.  */
	la	$sp, _stack
	movi	$r0, #-8		/* Set $r0 as 0xFFFFFFF8.  */
	and	$sp, $sp, $r0

.L_bss_clear:
        /* Clear bss section.  */
        la      $r3, _end
        la      $r0, _edata
	beq	$r0, $r3, .L_call_main	/* Branch if no bss.  */
        sub     $r1, $r3, $r0           /* Size to be clear.  */

        /* Set $r2 as how many words to be clear.
           Set $r1 as how many bytes are less than a woard to be clear.  */
        srli    $r2, $r1, #2
        andi    $r1, $r1, #3
        beqz    $r3, .Lbyte_clear

        la      $r4, #0x00000000
.Lword_clear:
        swi.bi  $r4, [$r0], #4
        addi    $r2, $r2, #-1
        bnez    $r2, .Lword_clear       /* Loop again ?  */
        beqz    $r1, .Lend_bss

.Lbyte_clear:
        sbi.bi  $r4, [$r0], #1
        addi    $r1, $r1, #-1
        bnez    $r1, .Lbyte_clear
.Lend_bss:


.L_call_main:
	la	$r15, main
	jral	$r15

.L_terminate_program:
	/* There are two ways to terminate program:
	    1. User "syscall 0x1" directly.
	    2. Call exit. The  return value $r0 from main() is
	      implicitly passed as argument.

	    Currently, we use option 2 as a solution to follow C99 5.1.2.2.3,
	    but aware that general exit() will do some cleanup procedures
	    which may result in large-memory-footprints.  */
	la	$r15, exit
	jral	$r15

.L_forever_loop:
	/* Should never return here.  */
        b       .L_forever_loop

	.size	_start, .-_start
