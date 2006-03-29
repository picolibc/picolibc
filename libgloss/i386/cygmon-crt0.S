/*
 * crt0 startup code for user programs running under Cygmon
 *
 * Copyright (c) 1998 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#ifndef NEED_UNDERSCORE
#define SYM(X) X
#else
#define SYM(X) _ ## X
#endif

.data
        .align  8
SYM(environ):
        .long   0

SYM(argc):
	.long	0

	.text
	.align 4

	.globl SYM(_start)
SYM(_start):
	/* see if the stack is already setup. if not, then default
         *  to using the value of %sp as set by the ROM monitor
         */
	lea __stack,%eax
	cmpl $0,%eax
	jz 1f
	mov %eax, %esp
1:
	mov $0, %ebp

	lea __bss_start, %eax
2:
	movb $0,(%eax)
	inc %eax
	cmp $__bss_end,%eax
	jl 2b
	
2:
	pushl $SYM(__sigtramp)
	pushl $0
	call SYM(__install_signal_handler)
	add $4, %esp

	pushl $SYM(__do_global_dtors)
	call SYM(atexit)
	add $4, %esp

	call SYM(__do_global_ctors)

	pushl $SYM(argc)
	call SYM(__get_program_arguments)
	add $4, %esp

	pushl %eax
	lea SYM(argc), %ebx
	pushl (%ebx)
	call SYM(main)
	add $8, %esp

	/* call exit from the C library so atexit gets called, and the
	 * C++ destructors get run. This calls our exit routine below	 
	 * when it's done.
	 */
	pushl %eax
	call	SYM(exit)
3:
	jmp 3b
