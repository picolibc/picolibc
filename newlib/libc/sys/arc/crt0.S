;; *******************************************************************************************************************
;; -----------------
;; ARC start up file
;; -----------------
;; The startup code for the ARC family of processors does the following before transferring control to user defined
;; _main label :
;;		1. Set sp to __stack_top (link time variable)
;;		2. Set fp to zero
;;		3. Zero out the bss section (for uninitialized globals)
;; After returning from main, the processor is halted and the pipeline is flushed out
;;
;; For Arctangent-A4, we do not support passing arguments to "main".
;; For subsequent processors, we expect arc in r0 and argv in r1.
;; These are saved in r13 / r14 during the initialization code.
;; *******************************************************************************************************************

	.file	"crt0.s"
	.extern main
	.text

	.global	__start
	.type	__start, @function

#ifdef __A4__
#define mov_s mov

__start:
	mov	sp,@__stack_top	; initialize stack pointer
	sub	sp,sp,16	; allocate expected call frame
				; it is not required for ARCompact, but is OK.

;; Zero out the bytes in the bss section
__zerobss:
	mov	%r2, @__bss_start
	mov	%r1, @_end
	sub	%r0, %r0, %r0	; use r0 as the zero value

	.align 4
__zbss1:
	stb	%r0, [%r2]	; write zero to _bss
	add	%r2, %r2, 1	; increment r2
	sub.f	0, %r2, %r1	; subtract to compare
	nop			; clear pipeline
	blo	@__zbss1		; branch if below

;; ifdef __A4__	
#else			

;; Startup code for the A5, ARC600 and ARC700 processors
;; -----------------------------------------------------
;; NOTE:  The following restrictions apply on zero overhead loops (other restrictions are not pertinent to this code)
;;	  -  loop end should be 4 instruction words away from the lp_count setting instruction
;;	  -  loop body should have at least two instruction words
__start:
	mov gp, @__SDATA_BEGIN__
	mov_s %r2, @__sbss_start		; r2 = start of the bss section
#if 1
	sub r3, @_end, %r2		; r3 = size of the bss section in bytes
#ifdef __A5__
	asr	r3, r3
	asr.f lp_count, r3		; set up the loop counter register to the size (in words) of the bss section
#else
	asr.f lp_count, r3, 2		; set up the loop counter register to the size (in words) of the bss section
#endif
#else
	mov.f lp_count,(@_end-@__sbss_start)/4 ; bss size in words
#endif
#if defined (__A5__) || defined (__A6__)
	lpnz @.Lend_zbss		; loop to zero out the bss. Enter loop only if lp_count != 0
	add r3,pcl,20
	sr r3,[2]			; LP_END
	mov sp, @__stack_top		; initialize stack pointer, and this instruction has 2 words
	mov_s r3,0
	st.ab r3,[%r2, 4]		; zero out the word
.Lend_zbss:
#else
	mov sp, @__stack_top		; initialize stack pointer, and this instruction has 2 words
	mov_s r3,0
	lpnz @.Lend_zbss		; loop to zero out the bss. Enter loop only if lp_count != 0
	st.ab r3,[%r2, 4]		; zero out the word
.Lend_zbss:
#endif	

;; ifdef __A4__
#endif

;; branch to main

/* Some  targets use the .init and .fini sections
   to create constructors and destructors, and for these
   targets we need to call the _init function and arrange
   for _fini to be called at program exit.  */
 	mov_s	r13, r0 
 	mov_s	r14, r1 
#if defined (_REENT_SMALL) || defined (__A4__)
#ifdef __A4__
 	mov	r0, %st(@_fini)
#else 
 	mov_s	r0, @_fini 
#endif	
 	bl	@atexit
#else
	; calling atexit drags in malloc, so instead poke the function
	; address directly into the reent structure
	ld	r1,[gp,@_impure_ptr@sda]
 	mov_s	r0, @_fini 
	add	r1,r1,0x14c	; &_GLOBAL_REENT->atexit0
	st	r1, [r1,-4]	; _GLOBAL_REENT->atexit
	st_s	r0, [r1,8]	; _GLOBAL_REENT->atexit0._fns[0]
	mov_s	r0, 1
	st_s	r0, [r1,4]	; _GLOBAL_REENT->atexit0._ind
#endif
	bl	@_init
	mov_s	r0, r13
	mov_s	r1, r14
	bl.d	@main
	mov	fp, 0		; initialize frame pointer
	; r0 contains exit code
	; we used to jump to exit_halt here, but that is wrong:
	; we have to run destructors and other things registered with atexit.
	j	@exit

	.global	_exit_halt
	.type	_exit_halt, @function

_exit_halt:
	; r0 contains exit code
	flag	0x01
	nop
	nop
	nop
	b	@_exit_halt
	nop
