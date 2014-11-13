/* Copyright (c) 2012-2013 Red Hat, Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use, modify,
   copy, or redistribute it subject to the terms and conditions of the BSD
   License.   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY expressed or implied, including the implied warranties
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  A copy of this license
   is available at http://www.opensource.org/licenses. Any Red Hat trademarks that
   are incorporated in the source code or documentation are not subject to the BSD
   License and may only be used or replicated with the express permission of
   Red Hat, Inc.
*/

#include "memmodel.h"

;; The linker links all .crt_* sections in asciibetical order at the
;; same place.  So, the four digits in .crt_NNNN determine the link
;; order, so, keep them in sequential order here.  The first two
;; digits are set here, the second two allow users to insert code
;; between code fragments here.

#if L0
	.section ".resetvec", "a"
__msp430_resetvec_hook:
	.word	__start

	.section ".crt_0000init", "ax", @progbits
	.refsym	__msp430_resetvec_hook
#ifdef MINRT
	.refsym	__crt0_call_just_main
#else
	.refsym	__crt0_call_init_then_main
#endif
	.global __start
__start:
	mov_	#__stack, R1

	;; Disable watchdog timer.
	MOV	#0x5a80, &0x15C
#endif

#if Lbss
	.section ".crt_0100bss", "ax", @progbits

	.global __crt0_init_bss
__crt0_init_bss:	
	
	mov_	#__bssstart, R12
	clr.w	R13
	mov.w	#__bsssize, R14
#ifdef __MSP430X_LARGE__
	clr.w	R15		; We assume that __bsssize is never > 64M
#endif
	call_	#memset
#endif

#ifdef __MSP430X_LARGE__
#if Lhigh_bss
;;  Note - this section is only included in the
;;  startup code of the application if it is needed.

	.section ".crt_0150high_bss", "ax", @progbits
	
	.global __crt0_init_high_bss
__crt0_init_high_bss:
	
	mov_	#llo(__high_bssstart), R12
	mov_	#lhi(__high_bssstart), R13
	mov.w	#llo(__high_bsssize), R14
	mov.w	#lhi(__high_bsssize), R15	
	call_	#memset
#endif /* Lhigh_bss */
#endif /* __MSP430X_LARGE__ */

#if Lmovedata
	.section ".crt_0200movedata", "ax", @progbits

	.global __crt0_movedata
__crt0_movedata:	
	
	mov_	#__datastart, R12
	mov_	#__romdatastart, R13

	;;  memmove and memcpy do not currently work when src == dst
	cmp_	R12, R13
	jeq	1f

	mov.w	#__romdatacopysize, R14
#ifdef __MSP430X_LARGE__
	clr.w	R15		; We assume that __romdatacopysize is never > 64M
#endif
	call_	#memmove
1:	
#endif

#if Lmain_minrt
	.section ".crt_0300main", "ax", @progbits
	.global	__crt0_call_just_main
__crt0_call_just_main:
	clr.w	R12		; Set argc == 0
	call_	#main
#endif

#if Lmain
	.section ".crt_0300main", "ax", @progbits
	.global	__crt0_call_init_then_main
__crt0_call_init_then_main:
	call_	#__msp430_init

	clr.w	R12		; Set argc == 0
	call_	#main
#endif

#if Lcallexit
	.section ".crt_0400main_exit", "ax", @progbits
	.global	__crt0_call_exit
__crt0_call_exit:
	call_	#_exit
#endif

;----------------------------------------

#ifndef MINRT
#if L0
	.section ".crt_0500main_init", "ax", @progbits
	.global	_msp430_run_init_array
	.type	_msp430_run_init_array,@function
_msp430_run_init_array:
	mov_	#__init_array_start, R4
	mov_	#__init_array_end, R5
	mov_	#PTRsz, R6
	br_	#_msp430_run_array

	.global	_msp430_run_preinit_array
	.type	_msp430_run_preinit_array,@function
_msp430_run_preinit_array:
	mov_	#__preinit_array_start, R4
	mov_	#__preinit_array_end, R5
	mov_	#PTRsz, R6
	br_	#_msp430_run_array

	.global	_msp430_run_fini_array
	.type	_msp430_run_fini_array,@function
_msp430_run_fini_array:
	mov_	#__fini_array_start, R4
	mov_	#__fini_array_end, R5
	mov_	#-PTRsz, R6
	br_	#_msp430_run_array

_msp430_run_array:
	cmp_	R4, R5
	jeq	_msp430_run_done
	mov_	@R4, R7
	add_	R6, R4
	call_	@R7
	br_	_msp430_run_array

_msp430_run_done:
	ret_

;----------------------------------------

	.section	.init,"ax"

	.global __msp430_init
__msp430_init:

	.section	.fini,"ax"

	.global __msp430_fini
__msp430_fini:
	call_	#_msp430_run_fini_array
	
#endif
#endif
