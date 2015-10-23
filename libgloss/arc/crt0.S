/*
   Copyright (c) 2015, Synopsys, Inc. All rights reserved.

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
       2.  Set fp to zero
       3.  Zero out the bss section (for uninitialized globals)
   After returning from main, the processor is halted and the pipeline is
   flushed out.

   We expect argc in r0 and argv in r1.  These are saved in r13 / r14 during
   the initialization code.
*/

	.file	"crt0.S"
	.extern main

#if defined (__EM__) || defined (__HS__)
	.section .ivt, "a", @progbits

; handler's name,          type,    number,name,   offset in IVT (hex/dec)
.word   __start           ; exception   0   program entry point  0x0     0
.word   memory_error      ; exception   1   memory_error         0x4     4
.word   instruction_error ; exception   2   instruction_error    0x8     8
.word   EV_MachineCheck   ; exception   3   EV_MachineCheck      0xC     12
.word   EV_TLBMissI       ; exception   4   EV_TLBMissI          0x10    16
.word   EV_TLBMissD       ; exception   5   EV_TLBMissD          0x14    20
.word   EV_ProtV          ; exception   6   EV_ProtV             0x18    24
.word   EV_PrivilegeV     ; exception   7   EV_PrivilegeV        0x1C    28
.word   EV_SWI            ; exception   8   EV_SWI               0x20    32
.word   EV_Trap           ; exception   9   EV_Trap              0x24    36
.word   EV_Extension      ; exception   10  EV_Extension         0x28    40
.word   EV_DivZero        ; exception   11  EV_DivZero           0x2C    44
.word   EV_DCError        ; exception   12  EV_DCError           0x30    48
.word   EV_Malignedr      ; exception   13  EV_Maligned          0x34    52
.word   _exit_halt        ; exception   14  unused               0x38    56
.word   _exit_halt        ; exception   15  unused               0x3C    60
.word   IRQ_Timer0        ; IRQ         16  Timer 0              0x40    64
.word   IRQ_Timer1        ; IRQ         17  Timer 1              0x44    68
.word   IRQ_18            ; IRQ         18                       0x48    72
.word   IRQ_19            ; IRQ         19                       0x4C    76
.word   IRQ_20            ; IRQ         20                       0x50    80


	.section .text.__startup, "ax", @progbits
#else
	.text
#endif

	.global	__start
	.type	__start, @function

#ifdef __ARC601__
; Startup code for the ARC601 processor
__start:
	mov	gp, @__SDATA_BEGIN__
	mov	sp, @__stack_top	; Point to top of stack
	mov	r5, 0			; Zero value
	mov_s	r2, @__sbss_start	; r2 = start of the bss section
	sub	r3, @_end, r2		; r3 = size of the bss section in bytes

	asr_s	r3, r3
	asr_s	r3, r3			; r3 = size of bss in words

.Lbss_loop:
	cmp	r3, 0xff		; Check for max lp_count
	mov.le	lp_count, r3
	mov.gt	lp_count, 0xff
	lpnz	2f			; Loop to zero bss
	st.ab	r5,[r2, 4]		; Write word of zeros
	nop
2:
	sub.f	r3, r3, 0xff		; Decrement word count
	jp	.Lbss_loop

#else	/* __ARC601__ */

; Startup code for the ARC600, ARC700 and ARCv2 processors
; NOTE:  The following restrictions apply on zero overhead loops (other
; restrictions are not pertinent to this code)
; - loop end should be 4 instruction words away from the lp_count setting
;   instruction
; - loop body should have at least two instruction words
__start:
#if defined (__HS__)
	; Allow unaligned accesses.
	lr	r2, [0xA]
	bset	r2, r2, 19
	flag	r2
#endif
	mov	gp, @__SDATA_BEGIN__
	mov_s	r2, @__sbss_start	; r2 = start of the bss section
	sub	r3, @_end, r2		; r3 = size of the bss section in bytes
	; set up the loop counter register to the size (in words) of the bss section
	asr.f	lp_count, r3, 2
#if defined (__ARC600__)
	; loop to zero out the bss.  Enter loop only if lp_count != 0
	lpnz	@.Lend_zbss
	add	r3, pcl, 20
	sr	r3, [2]			; LP_END
	; initialize stack pointer, and this instruction has 2 words
	mov	sp, @__stack_top
	mov_s	r3, 0
	st.ab	r3, [r2, 4]		; zero out the word
.Lend_zbss:
#else
	mov	sp, @__stack_top	; initialize stack pointer
	mov_s	r3,0
	; loop to zero out the bss.  Enter loop only if lp_count != 0
	lpnz	@.Lend_zbss
	st.ab	r3,[r2, 4]		; zero out the word
	nop
.Lend_zbss:
#endif

#endif /* !__ARC601__ */

; Some  targets use the .init and .fini sections to create constructors and
; destructors, and for these targets we need to call the _init function and
; arrange for _fini to be called at program exit.
	mov_s	r13, r0
	mov_s	r14, r1
	; calling atexit drags in malloc, so instead poke the function
	; address directly into the reent structure
	ld	r1, [gp, @_impure_ptr@sda]
	mov_s	r0, @_fini
	add	r1, r1, 0x14c		; &_GLOBAL_REENT->atexit0
	st	r1, [r1, -4]		; _GLOBAL_REENT->atexit
	st_s	r0, [r1, 8]		; _GLOBAL_REENT->atexit0._fns[0]
	mov_s	r0, 1
	st_s	r0, [r1, 4]		; _GLOBAL_REENT->atexit0._ind
; branch to _init
#if defined (__EM__) || defined (__HS__)
	jl	@_init
#else
	bl	@_init
#endif
	mov_s	r0, r13
	mov_s	r1, r14
; branch to main
#if defined (__EM__) || defined (__HS__)
	mov	fp,0			; initialize frame pointer
	jl	@main
#else
	bl.d	@main
	mov	fp, 0			; initialize frame pointer
#endif
	; r0 contains exit code
	j	@exit

#if defined (__EM__) || defined (__HS__)
; ARCv2 default interrupt routines, defined as weak symbols.
; Default implementation halts the core.  To conserve code size those symbols
; share a single implementation, however as a downside debugger and
; disassembler will not be able to distinguish one from another.
.weak   memory_error
.weak   instruction_error
.weak   EV_MachineCheck
.weak   EV_TLBMissI
.weak   EV_TLBMissD
.weak   EV_ProtV
.weak   EV_PrivilegeV
.weak   EV_SWI
.weak   EV_Trap
.weak   EV_Extension
.weak   EV_DivZero
.weak   EV_DCError
.weak   EV_Malignedr
.weak   IRQ_Timer0
.weak   IRQ_Timer1
.weak   IRQ_18
.weak   IRQ_19
.weak   IRQ_20

.balign 4
memory_error        :
instruction_error   :
EV_MachineCheck     :
EV_TLBMissI         :
EV_TLBMissD         :
EV_ProtV            :
EV_PrivilegeV       :
EV_SWI              :
EV_Trap             :
EV_Extension        :
EV_DivZero          :
EV_DCError          :
EV_Malignedr        :
IRQ_Timer0          :
IRQ_Timer1          :
IRQ_18              :
IRQ_19              :
IRQ_20              :
.Lloop_halt:
	flag	0x01
	nop
	b	.Lloop_halt
	nop
#endif

	.section .text._exit_halt,"ax",@progbits
	.global	 _exit_halt
	.type	 _exit_halt, @function

_exit_halt:
	; r0 contains exit code
	flag	0x01
	nop
	nop			; ARCompact requires 3 nops after flag 1
	nop
	b	@_exit_halt
	nop
