/* Copyright (c) 2012-2015 Red Hat, Inc. All rights reserved.

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
;; same place.  So, the four digits in .crt_NNNN_xxx name created by
;; the START_CRT_FUNC macro determine the link order, so, keep them
;; in sequential order here.  The first two digits are set here, the
;; second two allow users to insert code between code fragments here.

#if L0
	.section ".resetvec", "a"
__msp430_resetvec_hook:
	.word	__crt0_start

	;; Here we provide weak definitions of the symbols used in the
	;; init_highbss and move_highdata blocks, in case they are not
	;; provided by the linker script.  They are defined here because
	;; this block is always included in every executable, and because
	;; if there were defined in the blocks that need them their values
	;; would be used without giving the linker script a chance to
	;; override them.
	;; 
	;; The weak definitions are needed if the user targets an MCU
	;; without high memory - and hence uses a linker script without
	;; a definition of the .upper.bss or .upper.data sections - and
	;; they have compiled their code with the -mdata-region=either
	;; command line option.  That option causes the assembler to
	;; define the __crt0_move_highdata and/or crt0_init_highbss
	;; symbols, which in turn forces the inclusion of the
	;; move_highdata and/or init_highbss blocks in the startup code,
	;; regardless of the fact that the sections are not present in
	;; the linker script.

	WEAK_DEF __upper_data_init
	WEAK_DEF __rom_highdatacopysize
	WEAK_DEF __high_datastart
	WEAK_DEF __rom_highdatastart
	WEAK_DEF __high_bssstart
	WEAK_DEF __high_bsssize

START_CRT_FUNC 0000 start
	.refsym	__msp430_resetvec_hook
#ifdef MINRT
	.refsym	__crt0_call_just_main
#else
	.refsym	__crt0_call_init_then_main
#endif
	mov_	#__stack, R1

END_CRT_FUNC	start
#endif


#if Lbss
;; Note - this section is only included in the startup code of the
;; application if it is needed.  It is responsible for initializing
;; the contents of the .bss section.

START_CRT_FUNC 0100 init_bss

	mov_	#__bssstart, R12
	clr.w	R13
	mov_	#__bsssize, R14
#ifdef __MSP430X_LARGE__
	clr.w	R15		; We assume that __bsssize is never > 64K
#endif
	call_	#memset

END_CRT_FUNC	init_bss
#endif /* Lbss */


#ifdef __MSP430X_LARGE__
#if Lhigh_bss
;; Note - this section is only included in the startup code of the
;; application if it is needed.  It is responsible for initializing
;; the contents of the .upper.bss section.

START_CRT_FUNC 0200 init_highbss
	
	mov_	#__high_bssstart, R12
	mov.w	#0, R13
	mov_	#__high_bsssize, R14
	;; If __high_bsssize is zero then skip the call to memset.
	;; This can happen if all of the bss data was placed into .either.bss.
	cmp.w	#0, R14
	jeq	1f
	call_	#memset
1:
END_CRT_FUNC	init_highbss
#endif /* Lhigh_bss */
#endif /* __MSP430X_LARGE__ */


#if Lmovedata
;; Note - this section is only included in the startup code of the
;; application if it is needed.  It is responsible for copying the
;; contents of the .data section from its load address (in ROM) to
;; its run-time address (in RAM).

START_CRT_FUNC 0300 movedata

	mov_	#__datastart, R12
	mov_	#__romdatastart, R13

	;;  memmove and memcpy do not currently work when src == dst
	cmp_	R12, R13
	jeq	1f

	mov_	#__romdatacopysize, R14

	call_	#memmove
1:
END_CRT_FUNC	movedata
#endif /* Lmovedata  */


#ifdef __MSP430X_LARGE__
#if Lmove_highdata
;; Note - this section is only included in the startup code of the application
;; if it is needed.  It is responsible either for making sure that the
;; contents of the .upper.data section have their correct startup values.
;; If a copy of the .upper.data section is stored in ROM then this means
;; copying the contents into HIFRAM.  If a copy of .upper.data is stored in a
;; shadow section in HIFRAM then this means copying from the shadow section
;; into the real section.

START_CRT_FUNC 0400 move_highdata
	;;  __rom_highdatacopysize may be zero.  Test this first because
	;; its value may come from the weak definitions above and we do
	;; not want to access the memory at address 0 pointed to by the
	;; weak definition of __upper_data_init.
	mov.w	#__rom_highdatacopysize, R14
	cmp.w	#0, R14
	jeq	3f

	/* Test our status word.  */
	cmpx.w  #0, &__upper_data_init
	jeq	1f
	/* Status word is non-zero - copy from shadow into upper.  */
	mov_	#__high_datastart, R12
	mov_	#__rom_highdatastart, R13
	jmp	2f

1:	/* Status word is zero.  Copy from upper to shadow and change status word.  */
	movx.w  #1, &__upper_data_init
	mov_	#__rom_highdatastart, R12
	mov_	#__high_datastart, R13

2:	;; __rom_highdatacopysize may be zero.  memmove should cope.
	mov.w	#__rom_highdatacopysize, R14

	call_	#memmove
3:
END_CRT_FUNC	move_highdata
#endif /* Lmove_highdata */
#endif /* __MSP430X_LARGE__ */


#if Lmain_minrt
;; Note - this section is only included in the startup code of the
;; application if it is needed.  It is responsible for just calling
;; main.  No initialization code is called first, and main is not
;; expected to return.

START_CRT_FUNC 0600 call_just_main

	clr.w	R12		; Set argc == 0
	call_	#main
END_CRT_FUNC	call_just_main
#endif /* Lmain_minrt */


#if Lmain
;; Note - this section is only included in the startup code of the
;; application if it is needed.  It is responsible for calling the
;; initialization code - constructors, etc - and then main.  If main
;; returns then the following section should be present to catch it.

START_CRT_FUNC 0700 call_init_then_main

	call_	#__msp430_init

	clr.w	R12		; Set argc == 0
	call_	#main

END_CRT_FUNC	call_init_then_main
#endif /* Lmain */


#if Lcallexit
;; Note - this section is only included in the startup code of the
;; application if it is needed.  It is responsible for calling exit
;; once main has finished.

START_CRT_FUNC 0800 call_exit

	call_	#_exit

END_CRT_FUNC	call_exit
#endif /* Lcallexit  */

;----------------------------------------

#ifndef MINRT
#if L0
	.section ".crt_0900main_init", "ax", @progbits
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
#endif /* not MINRT */
